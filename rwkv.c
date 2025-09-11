
#include <glib.h>
#include <gmodule.h>
#include <moduler-model.h>
#include <rwkv_tokenizer.h>

#include <ctype.h>

#include "chatbot.h"

typedef struct _LMRealModule LMRealModule;

G_DEFINE_QUARK (rwkv-module-error, rwkv_module_error);
#define RWKV_MODULE_ERROR rwkv_module_error_quark ()

static void
g_string_strip (GString *str)
{
  size_t t;
  size_t e;
  g_return_if_fail (str);

  for (t = str->len; t != 0; t--)
    if (!isspace (str->str[t - 1]))
      break;
  for (e = 0; e < str->len; e++)
    if (!isspace (str->str[e]))
      break;

  g_string_truncate (str, t);
  g_string_erase (str, 0, e);
}

typedef struct _Args
{
  gchar *model;
  gchar *tokenizer;
  gchar *state_file;
} Args;

/* Parse key0=value0:key1=value1... style arguments */
static Args *
args_new (const gchar *param)
{
  Args *args;
  gchar **kv_array;

  args = g_new0 (Args, 1);

  kv_array = g_strsplit (param, ":", 0);
  for (size_t i = 0; kv_array[i] != NULL; i++)
    {
      gchar **kv;
      gchar *k;
      gchar *v;

      kv = g_strsplit (kv_array[i], "=", 2);
      if (g_strv_length (kv) != 2)
        {
          g_warning ("Given parameter '%s' is not key=value form.",
                     kv_array[i]);
          g_strfreev (kv);
          continue;
        }

      k = kv[0];
      v = kv[1];

      if (g_str_equal (k, "model") && (args->model == NULL))
        args->model = g_strdup (v);
      if (g_str_equal (k, "tokenizer") && (args->tokenizer == NULL))
        args->tokenizer = g_strdup (v);
      if (g_str_equal (k, "state_file") && (args->state_file == NULL))
        args->state_file = g_strdup (v);

      g_strfreev (kv);
    }

  if (args->model == NULL)
    goto on_error;
  if (args->tokenizer == NULL)
    goto on_error;

  g_strfreev (kv_array);
  return args;
on_error:
  g_warning ("Required parameter(s) is/are not provided.\n"
             "You need:\n"
             "model=[path to the onnx file]\n"
             "tokenizer=[path to the txt file]\n");

  g_free (args->state_file);
  g_free (args->tokenizer);
  g_free (args->model);
  g_free (args);
  return NULL;
}

static void
args_free (Args *args)
{
  g_return_if_fail (args);

  g_free (args->state_file);
  g_free (args->tokenizer);
  g_free (args->model);
  g_free (args);
}

/*
 * For normal RWKV, need fix if want to support vision, hearing, etc...
 * RWKV has input x, output y (logits), and internal state
 * Internal state consists of time-shift for time-mix and channel-mix, and WKV
 * state Because ONNX Runtime doesn't "restrict" input_values/output_values,
 * internal state are handled like in-place operation
 */
static GPtrArray *
create_value_array (MMContext *context, MMModel *model, GError **error)
{
  GPtrArray *values;
  MMValue *value = NULL;
  gboolean x = FALSE;
  gboolean y = FALSE;
  size_t nlayer = 0;

  values = g_ptr_array_new_full (model->input_infos->len
                                     + model->output_infos->len,
                                 (GDestroyNotify)mm_value_unref);

  for (size_t k = 0; k < model->input_infos->len; k++)
    {
      size_t layer_id;
      MMValueInfo *vinfo = model->input_infos->pdata[k];

      if (!strcmp (vinfo->name, "x") && !x)
        {
          value = mm_value_new (context, vinfo, model, "x", NULL, NULL, error);
          if (value == NULL)
            goto on_error;
          x = TRUE;
        }
      else if ((sscanf (vinfo->name, "x_tmix_last_%zd", &layer_id) == 1)
               && (nlayer == layer_id))
        {
          gchar *output_name = g_strdup_printf ("x_tmix_next_%zd", layer_id);
          value = mm_value_new (context, vinfo, model, vinfo->name,
                                output_name, NULL, error);
          g_free (output_name);
          nlayer++;
        }
      else if ((sscanf (vinfo->name, "wkv_state_%zd", &layer_id) == 1)
               && ((nlayer - 1) == layer_id))
        {
          gchar *output_name = g_strdup_printf ("wkv_next_%zd", layer_id);
          value = mm_value_new (context, vinfo, model, vinfo->name,
                                output_name, NULL, error);
          g_free (output_name);
        }
      else if ((sscanf (vinfo->name, "x_cmix_last_%zd", &layer_id) == 1)
               && ((nlayer - 1) == layer_id))
        {
          gchar *output_name = g_strdup_printf ("x_cmix_next_%zd", layer_id);
          value = mm_value_new (context, vinfo, model, vinfo->name,
                                output_name, NULL, error);
          g_free (output_name);
        }
      else
        goto on_error;

      if (value == NULL)
        goto on_error;
      g_ptr_array_add (values, value);
      value = NULL;
    }

  for (size_t k = 0; k < model->output_infos->len; k++)
    {
      MMValueInfo *vinfo = model->output_infos->pdata[k];
      MMValue *value = NULL;

      if (!strcmp (vinfo->name, "y") && !y)
        {
          value = mm_value_new (context, vinfo, model, NULL, "y", NULL, error);
          if (value == NULL)
            goto on_error;
          y = TRUE;
        }

      if (value)
        g_ptr_array_add (values, value);
    }

  if (!x || !y)
    goto on_error;

  return values;
on_error:
  mm_value_unref (value);
  g_ptr_array_unref (values);
  return NULL;
}

struct _LMRealModule
{
  LMModule public;

  RWKVTokenizer tokenizer;
  MMContext *context;
  MMModel *model;
  GPtrArray *value_array;
  MMModelInput *input;
  MMModelOutput *output;
  MMModelOutput *prompt_output;

  MMValue *idx;
  MMValue *out_idx;

  MMFile *state_file;
};

static void
dispose (LMModule *module)
{
  LMRealModule *rmodule = (LMRealModule *)module;
  g_return_if_fail (rmodule);

  if (rmodule->state_file)
    {
      GError *error = NULL;
      if (!mm_file_write (rmodule->state_file, &error))
        {
          if (error)
            {
              g_warning ("Failed to save state: %s\n", error->message);
              g_error_free (error);
            }
          else
            g_warning ("Failed to save state.\n");
        }
      mm_file_unref (rmodule->state_file);
    }
  mm_value_unref (rmodule->out_idx);
  mm_value_unref (rmodule->idx);
  mm_model_output_unref (rmodule->prompt_output);
  mm_model_output_unref (rmodule->output);
  mm_model_input_unref (rmodule->input);
  g_ptr_array_unref (rmodule->value_array);
  mm_model_unref (rmodule->model);
  mm_context_unref (rmodule->context);
  rwkv_tokenizer_free (&rmodule->tokenizer);
  g_free (rmodule);
}

static GString *
chat_template (const char *role, const char *message, ...)
{
  GString *result;
  GString *stripped_message;
  va_list args;

  g_return_val_if_fail (role, NULL);

  result = g_string_new (NULL);
  g_string_printf (result, "%s:", role);

  if (message == NULL)
    return result;
  stripped_message = g_string_new (message);
  g_string_strip (stripped_message);
  g_string_append_printf (result, " %s", stripped_message->str);

  va_start (args, message);

  while (TRUE)
    {
      const char *vrole;
      const char *vmessage;

      vrole = va_arg (args, const char *);
      if (vrole == NULL)
        break;
      g_string_append_printf (result, "\n\n%s:", vrole);

      g_string_erase (stripped_message, 0, -1);
      vmessage = va_arg (args, const char *);
      if (vmessage == NULL)
        break;
      g_string_append (stripped_message, vmessage);
      g_string_strip (stripped_message);

      g_string_append (result, stripped_message->str);
    }

  va_end (args);
  g_string_free (stripped_message, TRUE);

  return result;
}

static gboolean
prompt (LMModule *module, GString *str, GError **error)
{
  LMRealModule *rmodule = (LMRealModule *)module;
  size_t token_len = 0;
  int64_t *tokens = NULL;
  GHashTable *shape = NULL;
  int64_t batch_size = 1;

  tokens = rwkv_tokenizer_tokenize (&rmodule->tokenizer, str->str, &token_len);
  if (tokens == NULL)
    {
      g_set_error (error, RWKV_MODULE_ERROR, -1,
                   "Failed to tokenize given string '%s'", str->str);
      return FALSE;
    }

  shape = g_hash_table_new (g_str_hash, g_str_equal);
  g_hash_table_replace (shape, "batch", &batch_size);
  g_hash_table_replace (shape, "seq", &token_len);

  if (!mm_model_input_set_dimension (rmodule->input, shape, error))
    goto on_error;
  if (!mm_model_output_set_dimension (rmodule->prompt_output, shape, error))
    goto on_error;

  if (!mm_value_set_data (rmodule->idx, tokens, error))
    goto on_error;

  if (!mm_model_run (rmodule->model, rmodule->input, rmodule->prompt_output,
                     error))
    goto on_error;

  g_hash_table_unref (shape);
  free (tokens);
  return TRUE;
on_error:
  g_hash_table_unref (shape);
  free (tokens);
  return FALSE;
}

static GString *
inference (LMModule *module, GString *str, LMModuleCallback callback,
           gpointer user_data, GError **error)
{
  LMRealModule *rmodule = (LMRealModule *)module;
  size_t token_len = 0;
  int64_t *tokens = NULL;
  GHashTable *shape = NULL;
  int64_t batch_size = 1;
  int64_t sequence_length = 1;
  int64_t *token;
  char *tk_str;
  GString *generated;
  gboolean decode = TRUE;

  tokens = rwkv_tokenizer_tokenize (&rmodule->tokenizer, str->str, &token_len);
  if (tokens == NULL)
    {
      g_set_error (error, RWKV_MODULE_ERROR, -1,
                   "Failed to tokenize given string '%s'", str->str);
      return NULL;
    }

  shape = g_hash_table_new (g_str_hash, g_str_equal);
  g_hash_table_replace (shape, "batch", &batch_size);
  g_hash_table_replace (shape, "seq", &token_len);
  generated = g_string_new (NULL);

  if (!mm_model_input_set_dimension (rmodule->input, shape, error))
    goto on_error;
  if (!mm_model_output_set_dimension (rmodule->output, shape, error))
    goto on_error;

  if (!mm_value_set_data (rmodule->idx, tokens, error))
    goto on_error;
  free (tokens);

  if (!mm_model_run (rmodule->model, rmodule->input, rmodule->output, error))
    goto on_error;

  token = mm_value_get_data (rmodule->out_idx, error);
  if (token == NULL)
    goto on_error;
  token += rmodule->out_idx->info->dim[1] - 1;
  tk_str = rwkv_tokenizer_detokenize (&rmodule->tokenizer, token, 1);
  if (tk_str == NULL)
    goto on_error;
  callback (tk_str, user_data);
  g_string_append (generated, tk_str);
  free (tk_str);

  /* prepare for decoding */
  g_hash_table_replace (shape, "seq", &sequence_length);
  if (!mm_model_input_set_dimension (rmodule->input, shape, error))
    goto on_error;
  if (!mm_model_output_set_dimension (rmodule->output, shape, error))
    goto on_error;

  do
    {
      if (!mm_value_set_data (rmodule->idx, token, error))
        goto on_error;

      if (!mm_model_run (rmodule->model, rmodule->input, rmodule->output,
                         error))
        goto on_error;

      token = mm_value_get_data (rmodule->out_idx, error);
      if (token == NULL)
        goto on_error;

      if (*token == 0)
        break;
      tk_str = rwkv_tokenizer_detokenize (&rmodule->tokenizer, token, 1);
      if (tk_str == NULL)
        goto on_error;
      callback (tk_str, user_data);
      g_string_append (generated, tk_str);
      if (!strcmp (generated->str + (generated->len - 2), "\n\n"))
        decode = FALSE;
      free (tk_str);
    }
  while (decode);

  g_hash_table_unref (shape);
  return generated;
on_error:
  g_string_free (generated, TRUE);
  g_hash_table_unref (shape);
  return NULL;
}

typedef struct _ThinkCallbackData
{
  GString *thought;
  GString *generated;
  LMModuleCallback callback;
  LMModuleCallback think_callback;
  gpointer user_data;
} ThinkCallbackData;

static void
inference_with_think_callback (char *str, gpointer data)
{
  ThinkCallbackData *think_data = data;

  if (think_data->generated)
    {
      if (think_data->callback)
        think_data->callback (str, think_data->user_data);
      g_string_append (think_data->generated, str);
    }
  else
    {
      g_string_append (think_data->thought, str);

      if ((think_data->thought->len > 8)
          && !strcmp (think_data->thought->str - 8, "</think>"))
        {
          think_data->generated = g_string_new (NULL);
          g_string_truncate (think_data->thought,
                             8); // 8 is come from "</think>"
          g_string_strip (think_data->thought);
        }
      else
        {
          if (think_data->think_callback)
            think_data->think_callback (str, think_data->user_data);
        }
    }
}

static GString *
inference_with_think (LMModule *module, GString *str, GString **thought,
                      LMModuleCallback think_callback,
                      LMModuleCallback callback, gpointer user_data,
                      GError **error)
{
  ThinkCallbackData data
      = { g_string_new (NULL), NULL, callback, think_callback, user_data };
  GString *str_modified = g_string_new (NULL);
  GString *generated;

  g_string_printf (str_modified, "%s <think", str->str);
  generated = inference (module, str_modified, inference_with_think_callback,
                         &data, error);
  g_string_free (generated, TRUE);
  if (thought)
    *thought = data.thought;
  else
    g_string_free (data.thought, TRUE);
  return data.generated;
}

G_MODULE_EXPORT LMModule *
lm_module_init (LMModuleParam *param, GError **error)
{
  LMRealModule *module;
  Args *args;
  MMModelOptions *options = NULL;
  bool has_state = FALSE;

  args = args_new (param->param_string);
  if (args == NULL)
    return NULL;

  module = g_new (LMRealModule, 1);

  module->public.type = LM_MODULE_TYPE_BASE;
  module->public.dispose = dispose;
  module->public.chat_template = chat_template;
  module->public.prompt = prompt;
  module->public.inference = inference;
  module->public.inference_with_think = inference_with_think;

  if (!rwkv_tokenizer_init (&module->tokenizer, args->tokenizer))
    goto on_error; // RWKV tokenizer need fix

  module->context = mm_context_new (error);
  if (module->context == NULL)
    goto on_error;

  options = mm_model_options_new (module->context, error);
  if (options == NULL)
    goto on_error;

  module->model = mm_model_new (options, args->model, error);
  if (module->model == NULL)
    goto on_error;

  module->value_array
      = create_value_array (module->context, module->model, error);
  if (module->value_array == NULL)
    goto on_error;

  module->input = mm_model_input_new (module->value_array);
  module->output = mm_model_output_new (module->value_array);
  module->prompt_output = mm_model_output_new (module->value_array);

  for (size_t k = 0; k < module->value_array->len; k++)
    {
      MMValue *v = module->value_array->pdata[k];
      if (!g_strcmp0 (v->input_name, "x") && (module->idx == NULL))
        {
          mm_value_ref (v);
          module->idx = v;
        }

      if (!g_strcmp0 (v->output_name, "y") && (module->out_idx == NULL))
        {
          mm_value_ref (v);
          module->out_idx = v;
        }
    }
  // mm_model_output_remove(module->prompt_output, module->out_idx);

  if (args->state_file)
    {
      module->state_file = mm_file_new (args->state_file);
      for (guint k = 0; k < module->value_array->len; k++)
        {
          MMValue *v = module->value_array->pdata[k];
          if (g_strcmp0 (v->input_name, "x")
              && g_strcmp0 (v->output_name, "y"))
            mm_file_add_value (module->state_file, v);
        }

      if (!mm_file_read (module->state_file, NULL))
        g_warning ("Failed to load state\n");
      else
        has_state = TRUE;
    }

  mm_model_options_unref (options);
  args_free (args);

  return (LMModule *)module;
on_error:
  mm_model_options_unref (options);

  mm_file_unref (module->state_file);
  mm_value_unref (module->idx);
  mm_value_unref (module->out_idx);
  mm_model_output_unref (module->prompt_output);
  mm_model_output_unref (module->output);
  mm_model_input_unref (module->input);
  g_ptr_array_unref (module->value_array);
  mm_model_unref (module->model);
  mm_context_unref (module->context);
  g_free (module);
  args_free (args);
  return NULL;
}
