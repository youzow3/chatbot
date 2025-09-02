#pragma once

#include <glib.h>

typedef enum _LMModuleType
{
	LM_MODULE_TYPE_BASE = 0,
	LM_MODULE_TYPE_MAX
} LMModuleType;

typedef struct _LMModuleParam LMModuleParam;
typedef struct _LMModule LMModule;

typedef void (*LMModuleCallback)(char *token_str);

struct _LMModuleParam
{
	char *model;
};

/*
 * LMModule - chatbot module for base language model
 *
 * The module writer should define following function:
 *
 * LMModule *init(LMModuleParam *param, GError **error);
 *
 * Where:
 * param: module specific parameters (Like specifing parameter size, or model name)
 * error: pointer to the GError *, which is used to report an error (Not programming error described in GLib documents
 */
struct _LMModule
{
	/* module type, BASE is only valid in this version */
	LMModuleType type;
	/* Free up all resources allocated/owned by this module */
	void (*dispose)(LMModule *module);
	/*
	 * generate chat template from given data
	 * argments should end with NULL, and message part can be NULL
	 * if message is NULL, this function should return string like "...\n<role>:"
	 */
	GString *(*chat_template)(const char *role, const char *message, ...);
	/*
	 * generate string from given string. This should follow chat template
	 * callback should be called everytime a token is generated.
	 */
	GString *(*inference)(LMModule *module, GString *str, LMModuleCallback callback, GError **error);
	/*
	 * same as inference(), but the model should "think"
	 * return value should be a generated text without model's thought
	 * if thought is not NULL, GString will be allocated, and put model's thought
	 * if the model didn't think, think_callback won't be called, and thought won't be modified
	 */
	GString *(*inference_with_think)(LMModule *module, GString *str, GString **thought, LMModuleCallback think_callback, LMModuleCallback callback, GError **error);
};
