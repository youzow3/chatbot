
#include "chatbot.h"

G_DEFINE_QUARK (chatbot-module-error-quark, chatbot_module_error);

LMModuleData *
lm_module_data_new (const gchar *path, const gchar *param, GError **error)
{
  LMModuleData *data;
  LMModuleParam lm_param;
  LMModule *(*init) (LMModuleParam *, GError **);

  data = g_new0 (LMModuleData, 1);
  data->module = g_module_open_full (path, G_MODULE_BIND_LAZY, error);
  if (data->module == NULL)
    goto on_error;

  if (!g_module_symbol (data->module, "lm_module_init", (gpointer *)&init))
    {
      g_set_error (error, CHATBOT_MODULE_ERROR, CHATBOT_MODULE_ERROR_NOT_FOUND,
                   "lm_module_init is not found.");
      goto on_error;
    }
  if (init == NULL)
    {
      g_set_error (error, CHATBOT_MODULE_ERROR, CHATBOT_MODULE_ERROR_INVALID,
                   "lm_module_init is found, but NULL.");
      goto on_error;
    }

  lm_param.param_string = param;
  data->lm = init (&lm_param, error);
  if (data->lm == NULL)
    goto on_error;

  return data;
on_error:
  g_module_close (data->module);
  g_free (data);
  return NULL;
}

void
lm_module_data_free (LMModuleData *module_data)
{
  g_return_if_fail(module_data);
  module_data->lm->dispose (module_data->lm);
  g_module_close (module_data->module);
  g_free (module_data);
}

ETModuleData *
et_module_data_new (const gchar *path, const gchar *param, GError **error)
{
  ETModuleData *data;
  ETModuleParam et_param;
  ETModule *(*init) (ETModuleParam *param, GError **error);

  data = g_new0 (ETModuleData, 1);

  data->module = g_module_open_full (path, G_MODULE_BIND_LAZY, error);
  if (data->module == NULL)
    goto on_error;

  et_param.param_string = param;

  if (!g_module_symbol (data->module, "et_module_init", (gpointer *)&init))
    {
      g_set_error (error, CHATBOT_MODULE_ERROR, CHATBOT_MODULE_ERROR_NOT_FOUND,
                   "et_module_init is not found.");
      goto on_error;
    }
  if (init == NULL)
    {
      g_set_error (error, CHATBOT_MODULE_ERROR, CHATBOT_MODULE_ERROR_INVALID,
                   "et_module_init is found, but NULL.");
      goto on_error;
    }

  data->et = init (&et_param, error);
  if (data->et == NULL)
    goto on_error;

  return data;
on_error:
  g_free (data);
  return NULL;
}

void
et_module_data_free (ETModuleData *module_data)
{
  g_return_if_fail(module_data);
  module_data->et->dispose (module_data->et);
  g_module_close (module_data->module);
  g_free (module_data);
}
