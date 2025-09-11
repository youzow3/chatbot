#pragma once

#include <gio/gio.h>
#include <glib.h>

typedef struct _LMModuleParam LMModuleParam;
typedef struct _LMModule LMModule;
typedef struct _LMModuleData LMModuleData;
typedef struct _ETModuleParam ETModuleParam;
typedef struct _ETModuleCommand ETModuleCommand;
typedef struct _ETModule ETModule;
typedef struct _ETModuleData ETModuleData;

typedef enum _ChatbotModuleError
{
  CHATBOT_MODULE_ERROR_UNDEFINED,
  CHATBOT_MODULE_ERROR_NOT_FOUND,
  CHATBOT_MODULE_ERROR_INVALID
} ChatbotModuleError;

#define CHATBOT_MODULE_ERROR chatbot_module_error_quark ()

typedef enum _LMModuleType
{
  LM_MODULE_TYPE_BASE = 0,
  LM_MODULE_TYPE_MAX
} LMModuleType;

typedef void (*LMModuleCallback) (char *token_str, gpointer user_data);

struct _LMModuleParam
{
  const char *param_string;
};

/*
 * LMModule - chatbot module for base language model
 *
 * The module writer should define following function:
 *
 * LMModule *init(LMModuleParam *param, GError **error);
 *
 * Where:
 * param: module specific parameters (Like specifing parameter size, or model
 * name) error: pointer to the GError *, which is used to report an error (Not
 * programming error described in GLib documents
 */
struct _LMModule
{
  /* module type, BASE is only valid in this version */
  LMModuleType type;
  /* Free up all resources allocated/owned by this module */
  void (*dispose) (LMModule *module);
  /*
   * generate chat template from given data
   * argments should end with NULL, and message part can be NULL
   * if message is NULL, this function should return string like "...\n<role>:"
   */
  GString *(*chat_template) (const char *role, const char *message, ...);
  /*
   * process prompts, not generate any tokens
   * mainly used for processing prompt
   */
  gboolean (*prompt) (LMModule *module, GString *str, GError **error);
  /*
   * generate string from given string. This should follow chat template
   * callback should be called everytime a token is generated.
   */
  GString *(*inference) (LMModule *module, GString *str,
                         LMModuleCallback callback, gpointer user_data,
                         GError **error);
  /*
   * same as inference(), but the model should "think"
   * return value should be a generated text without model's thought
   * if thought is not NULL, GString will be allocated, and put model's thought
   * if the model didn't think, think_callback won't be called, and thought
   * won't be modified
   */
  GString *(*inference_with_think) (LMModule *module, GString *str,
                                    GString **thought,
                                    LMModuleCallback think_callback,
                                    LMModuleCallback callback,
                                    gpointer user_data, GError **error);
};

struct _LMModuleData
{
  GModule *module;
  LMModule *lm;
};

LMModuleData *lm_module_data_new (const gchar *path, const gchar *param,
                                  GError **error);
void lm_module_data_free (LMModuleData *module_data);

struct _ETModuleParam
{
  const gchar *param_string;
};

struct _ETModuleCommand
{
  /*
   * Command description, this will feed to LLMs
   * description must not be NULL, and have enough information to run
   * sucessfully
   */
  const gchar *description;
  /*
   * Command's main function
   * Same as main function in C language except for existance of output
   *
   * output should be assigned if output is not NULL
   * You should consider main() as CLI application with restrictions
   * Restrictions are you can't use stdin, stdout, and stderr, and if you want
   * to do such things, like printf(), you should write to *output You can't
   * use stdin to interact with LLMs. If you use stdin include scanf(), this
   * will wait user's input.
   */
  int (*main) (ETModule *module, int argc, char **argv, GString *output);
};

/*
 * ETModule - external tool module
 *
 * The module writer should define following function:
 *
 * ETModule *init(ETModuleParam *param, GError **error);
 *
 * Where:
 * param: module specific parameters (Like specifing parameter size, or model
 * name) error: pointer to the GError *, which is used to report an error (Not
 * programming error described in GLib documents
 *
 */
struct _ETModule
{
  /*
   * Module name (namespace) it should be a unique name, thus if there was any
   * name duplication, first one is loaded and others are not loaded.
   */
  const gchar *name;
  /* Module description */
  const gchar *description;
  /*
   * key: gchar *, value: ETModuleCommand *
   * commands should be initialized at init() and never be changed
   */
  GHashTable *commands;
  /*
   * Dispose all resources allocated by this module
   * Called once, and all members are no longer accessed after dispose()
   */
  void (*dispose) (ETModule *module);
};

struct _ETModuleData
{
  GModule *module;
  ETModule *et;
};

ETModuleData *et_module_data_new (const gchar *path, const gchar *param,
                                  GError **error);
void et_module_data_free (ETModuleData *module_data);
