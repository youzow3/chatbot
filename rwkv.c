
#include <glib.h>
#include <gmodule.h>
#include <moduler-model.h>
#include <rwkv_tokenizer.h>

#include <ctype.h>

#include "chatbot.h"

typedef struct _LMRealModule LMRealModule;

G_DEFINE_QUARK(rwkv-module-error, rwkv_module_error);
#define RWKV_MODULE_ERROR rwkv_module_error_quark()

static void g_string_strip(GString *str) {
  size_t t;
  size_t e;
  g_return_if_fail(str);

  for (t = str->len; t != 0; t--)
    if (!isspace(str->str[t - 1]))
      break;
  for (e = 0; e < str->len; e++)
    if (!isspace(str->str[e]))
      break;

  g_string_truncate(str, t);
  g_string_erase(str, 0, e);
}

/*
 * For normal RWKV, need fix if want to support vision, hearing, etc...
 * RWKV has input x, output y (logits), and internal state
 * Internal state consists of time-shift for time-mix and channel-mix, and WKV
 * state Because ONNX Runtime doesn't "restrict" input_values/output_values,
 * internal state are handled like in-place operation
 */
static GPtrArray *create_value_array(MMContext *context, MMModel *model,
                                     GError **error) {
  GPtrArray *values;
  MMValue *value = NULL;
  gboolean x = FALSE;
  gboolean y = FALSE;
  size_t nlayer = 0;

  values =
      g_ptr_array_new_full(model->input_infos->len + model->output_infos->len,
                           (GDestroyNotify)mm_value_unref);

  for (size_t k = 0; k < model->input_infos->len; k++) {
    size_t layer_id;
    MMValueInfo *vinfo = model->input_infos->pdata[k];

    if (!strcmp(vinfo->name, "x") && !x) {
      value = mm_value_new(context, vinfo, model, "x", NULL, NULL, error);
      if (value == NULL)
        goto on_error;
      x = TRUE;
    } else if ((sscanf(vinfo->name, "x_tmix_last_%zd", &layer_id) == 1) &&
               (nlayer == layer_id)) {
      gchar *output_name = g_strdup_printf("x_tmix_next_%zd", layer_id);
      value = mm_value_new(context, vinfo, model, vinfo->name, output_name,
                           NULL, error);
      g_free(output_name);
      nlayer++;
    } else if ((sscanf(vinfo->name, "wkv_state_%zd", &layer_id) == 1) &&
               ((nlayer - 1) == layer_id)) {
      gchar *output_name = g_strdup_printf("wkv_next_%zd", layer_id);
      value = mm_value_new(context, vinfo, model, vinfo->name, output_name,
                           NULL, error);
      g_free(output_name);
    } else if ((sscanf(vinfo->name, "x_cmix_last_%zd", &layer_id) == 1) &&
               ((nlayer - 1) == layer_id)) {
      gchar *output_name = g_strdup_printf("x_cmix_next_%zd", layer_id);
      value = mm_value_new(context, vinfo, model, vinfo->name, output_name,
                           NULL, error);
      g_free(output_name);
    } else
      goto on_error;

    if (value == NULL)
      goto on_error;
    g_ptr_array_add(values, value);
    value = NULL;
  }

  for (size_t k = 0; k < model->output_infos->len; k++) {
    MMValueInfo *vinfo = model->output_infos->pdata[k];
    MMValue *value = NULL;

    if (!strcmp(vinfo->name, "y") && !y) {
      value = mm_value_new(context, vinfo, model, NULL, "y", NULL, error);
      if (value == NULL)
        goto on_error;
      y = TRUE;
    }

    if (value)
      g_ptr_array_add(values, value);
  }

  if (!x || !y)
    goto on_error;

  return values;
on_error:
  mm_value_unref(value);
  g_ptr_array_unref(values);
  return NULL;
}

struct _LMRealModule {
  LMModule public;

  RWKVTokenizer tokenizer;
  MMContext *context;
  MMModel *model;
  GPtrArray *value_array;
  MMModelInput *input;
  MMModelOutput *output;

  MMValue *idx;
  MMValue *out_idx;
};

static void dispose(LMModule *module) {
  LMRealModule *rmodule = (LMRealModule *)module;
  g_return_if_fail(rmodule);

  mm_value_unref(rmodule->out_idx);
  mm_value_unref(rmodule->idx);
  mm_model_output_unref(rmodule->output);
  mm_model_input_unref(rmodule->input);
  g_ptr_array_unref(rmodule->value_array);
  mm_model_unref(rmodule->model);
  mm_context_unref(rmodule->context);
  rwkv_tokenizer_free(&rmodule->tokenizer);
  g_free(rmodule);
}

static GString *chat_template(const char *role, const char *message, ...) {
  GString *result;
  GString *stripped_message;
  va_list args;

  g_return_val_if_fail(role, NULL);

  result = g_string_new(NULL);
  g_string_printf(result, "%s:", role);

  if (message == NULL)
    return result;
  stripped_message = g_string_new(message);
  g_string_strip(stripped_message);
  g_string_append_printf(result, " %s", stripped_message->str);

  va_start(args, message);

  while (TRUE) {
    const char *vrole;
    const char *vmessage;

    vrole = va_arg(args, const char *);
    if (vrole == NULL)
      break;
    g_string_append_printf(result, "\n\n%s:", vrole);

    g_string_erase(stripped_message, 0, -1);
    vmessage = va_arg(args, const char *);
    if (vmessage == NULL)
      break;
    g_string_append(stripped_message, vmessage);
    g_string_strip(stripped_message);

    g_string_append(result, stripped_message->str);
  }

  va_end(args);
  g_string_free(stripped_message, TRUE);

  return result;
}

static GString *inference(LMModule *module, GString *str,
                          LMModuleCallback callback, GError **error) {
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

  tokens = rwkv_tokenizer_tokenize(&rmodule->tokenizer, str->str, &token_len);
  if (tokens == NULL) {
    g_set_error(error, RWKV_MODULE_ERROR, -1,
                "Failed to tokenize given string '%s'", str->str);
    return NULL;
  }

  shape = g_hash_table_new(g_str_hash, g_str_equal);
  g_hash_table_replace(shape, "batch", &batch_size);
  g_hash_table_replace(shape, "seq", &token_len);
  generated = g_string_new(NULL);

  if (!mm_model_input_set_dimension(rmodule->input, shape, error))
    goto on_error;
  if (!mm_model_output_set_dimension(rmodule->output, shape, error))
	  goto on_error;

  if (!mm_value_set_data(rmodule->idx, tokens, error))
    goto on_error;
  free(tokens);

  if (!mm_model_run(rmodule->model, rmodule->input, rmodule->output, error))
    goto on_error;

  token = mm_value_get_data(rmodule->out_idx, error);
  if (token == NULL)
    goto on_error;
  token += rmodule->out_idx->info->dim[1] - 1;
  tk_str = rwkv_tokenizer_detokenize(&rmodule->tokenizer, token, 1);
  if (tk_str == NULL)
    goto on_error;
  callback(tk_str);
  g_string_append(generated, tk_str);
  free(tk_str);

  /* prepare for decoding */
  g_hash_table_replace(shape, "seq", &sequence_length);
  if (!mm_model_input_set_dimension(rmodule->input, shape, error))
    goto on_error;
  if (!mm_model_output_set_dimension(rmodule->output, shape, error))
    goto on_error;

  do {
    if (!mm_value_set_data(rmodule->idx, token, error))
      goto on_error;

    if (!mm_model_run(rmodule->model, rmodule->input, rmodule->output, error))
      goto on_error;

    token = mm_value_get_data(rmodule->out_idx, error);
    if (token == NULL)
      goto on_error;
    tk_str = rwkv_tokenizer_detokenize(&rmodule->tokenizer, token, 1);
    if (tk_str == NULL)
      goto on_error;
    callback(tk_str);
    g_string_append(generated, tk_str);
    if (!strcmp(tk_str, "\n\n"))
      decode = FALSE;
    free(tk_str);
  } while (decode);

  g_hash_table_unref(shape);
  return generated;
on_error:
  g_string_free(generated, TRUE);
  g_hash_table_unref(shape);
  return NULL;
}

static GString *inference_with_think(LMModule *module, GString *str,
                                     GString **thought,
                                     LMModuleCallback think_callback,
                                     LMModuleCallback callback,
                                     GError **error) {
  return inference(module, str, callback, error);
}

G_MODULE_EXPORT LMModule *init(LMModuleParam *param, GError **error) {
  LMRealModule *module;
  char *tokenizer_path = NULL;
  char *model_path = NULL;
  MMModelOptions *options = NULL;

  tokenizer_path = g_strdup_printf("%s.txt", param->model);
  model_path = g_strdup_printf("%s.onnx", param->model);

  module = g_new(LMRealModule, 1);

  module->public.dispose = dispose;
  module->public.chat_template = chat_template;
  module->public.inference = inference;
  module->public.inference_with_think = inference_with_think;

  if (!rwkv_tokenizer_init(&module->tokenizer, tokenizer_path))
    goto on_error; // RWKV tokenizer need fix

  module->context = mm_context_new(error);
  if (module->context == NULL)
    goto on_error;

  options = mm_model_options_new(module->context, error);
  if (options == NULL)
    goto on_error;

  module->model = mm_model_new(options, model_path, error);
  if (module->model == NULL)
    goto on_error;

  module->value_array =
      create_value_array(module->context, module->model, error);
  if (module->value_array == NULL)
    goto on_error;

  module->input = mm_model_input_new(module->value_array);
  module->output = mm_model_output_new(module->value_array);

  for (size_t k = 0; k < module->value_array->len; k++) {
    MMValue *v = module->value_array->pdata[k];
    if (!g_strcmp0(v->input_name, "x") && (module->idx == NULL)) {
      mm_value_ref(v);
      module->idx = v;
    }

    if (!g_strcmp0(v->output_name, "y") && (module->out_idx == NULL)) {
      mm_value_ref(v);
      module->out_idx = v;
    }
  }

  mm_model_options_unref(options);
  g_free(model_path);
  g_free(tokenizer_path);

  return (LMModule *)module;
on_error:
  g_free(model_path);
  g_free(tokenizer_path);
  mm_model_options_unref(options);

  mm_value_unref(module->idx);
  mm_value_unref(module->out_idx);
  mm_model_output_unref(module->output);
  mm_model_input_unref(module->input);
  g_ptr_array_unref(module->value_array);
  mm_model_unref(module->model);
  mm_context_unref(module->context);
  g_free(module);
  return NULL;
}
