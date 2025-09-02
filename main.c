
#include <glib.h>
#include <gmodule.h>

#include <stdio.h>

#include "chatbot.h"

enum
{
	ARGS_LM_MODULE,
	ARGS_LM_MODEL,
	ARGS_NULL
};

typedef struct _Args Args;

Args *args_new(int *argc, char ***argv, GError **error);
void args_free(Args *args);

typedef struct _Module Module;

Module *module_new(char *file_name, void *param, GError **error);
void module_free(Module *module);

struct _Args
{
	char *lm_module;
	char *lm_model;
};

Args *args_new(int *argc, char ***argv, GError **error)
{
	Args *args = NULL;
	GOptionContext *context = NULL;

	GOptionEntry entries[ARGS_NULL + 1];

	g_return_val_if_fail(argc, NULL);
	g_return_val_if_fail(argv, NULL);
	g_return_val_if_fail((error == NULL) || (*error == NULL), NULL);

	args = g_new0(Args, 1);
	entries[ARGS_LM_MODULE] = (GOptionEntry){"lm_module", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &args->lm_module, "LM-Module to use", "module"};
	entries[ARGS_LM_MODEL] = (GOptionEntry){"lm_model", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &args->lm_model, "Model to use, refer to LM-Module docs you use", "name"};
	entries[ARGS_NULL] = (GOptionEntry)G_OPTION_ENTRY_NULL;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, entries, NULL);

	if (!g_option_context_parse(context, argc, argv, error))
		goto on_error;

	g_option_context_free(context);

	return args;
on_error:
	g_option_context_free(context);
	g_free(args);
	return NULL;
}

void args_free(Args *args)
{
	g_return_if_fail(args);

	g_free(args->lm_model);
	g_free(args->lm_module);
	g_free(args);
}

struct _Module
{
	GModule *module;
	void *data;
};

Module *module_new(char *file_name, void *param, GError **error)
{
	Module *module;
	void *(*init)(void *param, GError **error);
	g_return_val_if_fail(file_name, NULL);
	g_return_val_if_fail((error == NULL) || (*error == NULL), NULL);

	module = g_new(Module, 1);
	module->module = g_module_open_full(file_name, G_MODULE_BIND_LAZY, error);
	if (module->module == NULL)
		goto on_error;

	if (!g_module_symbol(module->module, "init", (gpointer*)&init))
		goto on_error;

	module->data = init(param, error);
	if (module->data == NULL)
		goto on_error;

	return module;
on_error:
	g_module_close(module->module);
	g_free(module);
	return NULL;
}

void module_free(Module *module)
{
	g_return_if_fail(module);

	g_module_close(module->module);
	g_free(module);
}

GString *read_user_input(void)
{
	gboolean loop = TRUE;
	GString *str = g_string_new(NULL);

	while (loop)
	{
		char buf[BUFSIZ];
		char *lf;
		memset(buf, 0, BUFSIZ);
		fgets(buf, BUFSIZ, stdin);
		if (buf[BUFSIZ - 1] != 0)
		{
			g_string_append_len(str, buf, BUFSIZ);
			continue;
		}

		lf = strrchr(buf, '\n');
		if (lf)
		{
			g_string_append_len(str, buf, lf - buf);
			loop = FALSE;
		}
		else
		{
			g_string_append(str, buf);
		}
	}

	return str;
}

void callback(char *str)
{
	printf("%s", str);
	fflush(stdout);
}

int main(int argc, char **argv)
{
	Args *args = NULL;
	Module *lm_module = NULL;
	LMModuleParam lm_module_param;
	LMModule *lm = NULL;
	GError *error = NULL;

	args = args_new(&argc, &argv, &error);
	if (args == NULL)
		goto on_error;

	lm_module_param.model = args->lm_model;
	lm_module = module_new(args->lm_module, &lm_module_param, &error);
	if (lm_module == NULL)
		goto on_error;
	lm = lm_module->data;

	/* main loop */
	while (TRUE)
	{
		GString *user_input = NULL;
		GString *chat_template = NULL;
		GString *output = NULL;

		printf("\nUser: ");
		fflush(stdout);
		user_input = read_user_input();
		if (!strcmp(user_input->str, "!exit"))
		{
			g_string_free(user_input, TRUE);
			break;
		}

		chat_template = lm->chat_template("User", user_input->str, "Assistant", NULL);
		printf("\nAssistant: ");
		fflush(stdout);
		output = lm->inference(lm, chat_template, callback, &error);
		if (output == NULL)
			goto on_main_loop_error;

		g_string_free(output, TRUE);
		g_string_free(chat_template, TRUE);
		g_string_free(user_input, TRUE);
		continue;
on_main_loop_error:
		g_string_free(output, TRUE);
		g_string_free(chat_template, TRUE);
		g_string_free(user_input, TRUE);
		goto on_error;
	}

	lm->dispose(lm);
	module_free(lm_module);
	args_free(args);

	return 0;
on_error:
	if (error)
	{
		fprintf(stderr, "Fatal: %s\n", error->message);
		g_error_free(error);
	}

	if (lm)
		lm->dispose(lm);
	module_free(lm_module);
	args_free(args);
	return 1;
}
