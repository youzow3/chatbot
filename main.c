
#include <gio/gio.h>
#include <glib.h>
#include <gmodule.h>

#include <stdio.h>

#include "chatbot.h"

enum
{
  ARGS_LM_MODULE,
  ARGS_LM_MODEL,
  ARGS_LM_MODULE_PARAM,
  ARGS_SYSTEM_PROMPT,
  ARGS_SYSTEM_PROMPT_FILE,
  ARGS_CHAT_HISTORY,
  ARGS_SHOW_THINKING,
  ARGS_ET_MODULE,
  ARGS_ET_MODULE_PARAM,
  ARGS_NULL
};

typedef struct _Args Args;

static Args *args_new (int *argc, char ***argv, GError **error);
static void args_free (Args *args);

struct _Args
{
  char *lm_module;
  char *lm_model;
  char *lm_module_param;
  char *system_prompt;
  char *system_prompt_file;
  char *chat_history_file;
  char **et_module;
  char *et_module_param;
  gboolean show_thinking;
};

static Args *
args_new (int *argc, char ***argv, GError **error)
{
  Args *args = NULL;
  GOptionContext *context = NULL;

  GOptionEntry entries[ARGS_NULL + 1];

  g_return_val_if_fail (argc, NULL);
  g_return_val_if_fail (argv, NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);

  args = g_new0 (Args, 1);
  entries[ARGS_LM_MODULE] = (GOptionEntry){ "lm_module",
                                            0,
                                            G_OPTION_FLAG_NONE,
                                            G_OPTION_ARG_FILENAME,
                                            &args->lm_module,
                                            "LM-Module to use",
                                            "module" };
  entries[ARGS_LM_MODEL]
      = (GOptionEntry){ "lm_model",
                        0,
                        G_OPTION_FLAG_DEPRECATED,
                        G_OPTION_ARG_STRING,
                        &args->lm_model,
                        "Model to use, refer to LM-Module docs you use",
                        "name" };
  entries[ARGS_LM_MODULE_PARAM] = (GOptionEntry){ "lm_module_param",
                                                  0,
                                                  G_OPTION_FLAG_NONE,
                                                  G_OPTION_ARG_STRING,
                                                  &args->lm_module_param,
                                                  "Parameters for LMModule",
                                                  "param" };
  entries[ARGS_SYSTEM_PROMPT] = (GOptionEntry){ "system_prompt",
                                                0,
                                                G_OPTION_FLAG_NONE,
                                                G_OPTION_ARG_STRING,
                                                &args->system_prompt,
                                                "System prompt",
                                                "prompt" };
  entries[ARGS_SYSTEM_PROMPT_FILE] = (GOptionEntry){ "system_prompt_file",
                                                     0,
                                                     G_OPTION_FLAG_NONE,
                                                     G_OPTION_ARG_FILENAME,
                                                     &args->system_prompt_file,
                                                     "System prompt file",
                                                     "file" };
  entries[ARGS_CHAT_HISTORY] = (GOptionEntry){ "chat_history",
                                               0,
                                               G_OPTION_FLAG_NONE,
                                               G_OPTION_ARG_STRING,
                                               &args->chat_history_file,
                                               "Destination for chat history",
                                               "path" };
  entries[ARGS_ET_MODULE]
      = (GOptionEntry){ "et_module",        0,
                        G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME_ARRAY,
                        &args->et_module,   "External tool module(s) to use",
                        "modules" };
  entries[ARGS_ET_MODULE_PARAM]
      = (GOptionEntry){ "et_module_param",
                        0,
                        G_OPTION_FLAG_NONE,
                        G_OPTION_ARG_STRING,
                        &args->et_module_param,
                        "External tool module(s) parameters",
                        "param" };
  entries[ARGS_SHOW_THINKING]
      = (GOptionEntry){ "show_thinking",
                        0,
                        G_OPTION_FLAG_NONE,
                        G_OPTION_ARG_NONE,
                        &args->show_thinking,
                        "Show thinking when LLM thinks",
                        NULL };
  entries[ARGS_NULL] = (GOptionEntry)G_OPTION_ENTRY_NULL;

  context = g_option_context_new (NULL);
  g_option_context_add_main_entries (context, entries, NULL);

  if (!g_option_context_parse (context, argc, argv, error))
    goto on_error;

  g_option_context_free (context);

  return args;
on_error:
  g_option_context_free (context);
  g_free (args);
  return NULL;
}

static void
args_free (Args *args)
{
  g_return_if_fail (args);

  g_free (args->et_module_param);
  g_strfreev (args->et_module);
  g_free (args->chat_history_file);
  g_free (args->system_prompt_file);
  g_free (args->system_prompt);
  g_free (args->lm_module_param);
  g_free (args->lm_model);
  g_free (args->lm_module);
  g_free (args);
}

static GString *
generate_et_system_prompt (GHashTable *module_table)
{
  GHashTableIter i;
  gchar *name;
  ETModuleData *module;
  GString *prompt = g_string_new ("# External Tools\n");

  g_hash_table_iter_init (&i, module_table);
  while (g_hash_table_iter_next (&i, (gpointer *)&name, (gpointer *)&module))
    {
      GHashTableIter f;
      gchar *command_name;
      ETModuleCommand *command;

      g_string_append_printf (prompt, "## Module %s\n%s\n\n### Function(s)\n",
                              module->et->name, module->et->description);
      for (g_hash_table_iter_init (&f, module->et->commands);
           g_hash_table_iter_next (&f, (gpointer *)&command_name,
                                   (gpointer *)&command);)
        g_string_append_printf (prompt, "#### %s\n%s\n", command_name,
                                command->description);
    }

  g_string_append_printf (
      prompt, "You can use external tool by starting line with \'!\'.\n"
              "Thus, syntax is \"![command] [arg1] [arg2]  [arg3] ...\"\n");

  return prompt;
}

static GString *
read_user_input (void)
{
  gboolean loop = TRUE;
  GString *str = g_string_new (NULL);

  while (loop)
    {
      char buf[BUFSIZ];
      char *lf;
      memset (buf, 0, BUFSIZ);
      fgets (buf, BUFSIZ, stdin);
      if (buf[BUFSIZ - 1] != 0)
        {
          g_string_append_len (str, buf, BUFSIZ);
          continue;
        }

      lf = strrchr (buf, '\n');
      if (lf)
        {
          g_string_append_len (str, buf, lf - buf);
          loop = FALSE;
        }
      else
        {
          g_string_append (str, buf);
        }
    }

  return str;
}

/**
 * Scan the output, and if there are command calls, process them.
 * Returns command execution results. It should be processed by chat_template
 * when feed to LLMs.
 */
static GString *
process_command (GString *assistant_output, GHashTable *module_table)
{
  char *output_str = g_strdup (assistant_output->str);
  char *line = output_str;
  GString *output = g_string_new (NULL);
  GError *error = NULL;

  do
    {
      char *new_line;
      char *command;
      gint argc;
      gchar **argv;

      line = g_strchug (line);
      if (line[0] != '!')
        {
          line = strstr (line, "\n");
          if (line)
            line += 1;
          continue;
        }

      line += 1;
      new_line = strstr (line, "\n");
      if (new_line)
        {
          command = g_strndup (line, new_line - line);
          line = new_line + 1;
        }
      else
        {
          command = g_strdup (line);
          line = NULL;
        }

      if (!g_shell_parse_argv (command, &argc, &argv, &error))
        {
          g_string_append_printf (output,
                                  "Failed to parse command \"%s\": %s\n",
                                  command, error->message);
          g_clear_pointer (&error, g_error_free);
        }
      else
        {
          const char *namespace_sep = strstr (argv[0], "::");
          gchar *namespace = NULL;
          gchar *cmd_name = NULL;
          ETModuleCommand *etm_command = NULL;
          ETModuleData *tool = NULL;

          if (namespace_sep)
            {
              namespace = g_strndup (argv[0], namespace_sep - argv[0]);
              cmd_name = g_strdup (namespace_sep + 2);
            }
          else
            cmd_name = g_strdup (argv[0]);

          if (namespace)
            {
              tool = g_hash_table_lookup (module_table, namespace);
              if (tool == NULL)
                {
                  g_string_append_printf (output,
                                          "Failed to parse command: \"%s\": "
                                          "Module name \"%s\" is not found.\n",
                                          command, namespace);
                  goto command_not_found;
                }

              etm_command = g_hash_table_lookup (tool->et->commands, cmd_name);
            }
          else
            {
              GHashTableIter i;

              g_hash_table_iter_init (&i, module_table);
              while (g_hash_table_iter_next (&i, NULL, (gpointer *)&tool)
                     && (etm_command == NULL))
                etm_command
                    = g_hash_table_lookup (tool->et->commands, cmd_name);
            }

          if (etm_command == NULL)
            g_string_append_printf (
                output,
                "Failed to execute command \"%s\". Command not found.\n",
                command);
          else
            {
              int ret = etm_command->main (tool->et, argc, argv, output);
              g_string_append_printf (
                  output, "\nCommand \"%s\" is returned with code %d\n",
                  command, ret);
            }

        command_not_found:
          g_free (cmd_name);
          g_free (namespace);
        }

      g_free (command);
    }
  while (line);
  g_free (output_str);

  return output;
}

static void
callback (char *str, gpointer data)
{
  printf ("%s", str);
  fflush (stdout);
}

int
main (int argc, char **argv)
{
  Args *args = NULL;
  LMModuleData *lm_module = NULL;
  GString *system_prompt = NULL;
  GHashTable *et_modules = NULL;
  GString *et_system_prompt = NULL;
  GFile *chat_history_file = NULL;
  GFileOutputStream *chat_history_stream = NULL;
  GError *error = NULL;

  args = args_new (&argc, &argv, &error);
  if (args == NULL)
    goto on_error;

  lm_module
      = lm_module_data_new (args->lm_module, args->lm_module_param, &error);
  if (lm_module == NULL)
    goto on_error;

  if (args->system_prompt)
    system_prompt = g_string_new (args->system_prompt);
  else if (args->system_prompt_file)
    {
      char *contents = NULL;
      if (!g_file_get_contents (args->system_prompt_file, &contents, NULL,
                                &error))
        goto on_error;

      system_prompt = g_string_new_take (contents);
    }
  else
    {
      system_prompt = g_string_new (NULL);
      g_warning ("No system prompt was set.\n");
    }

  if (args->chat_history_file)
    {
      chat_history_file = g_file_new_for_path (args->chat_history_file);
      chat_history_stream = g_file_append_to (
          chat_history_file, G_FILE_CREATE_PRIVATE, NULL, &error);
      if (chat_history_stream == NULL)
        goto on_error;
    }

  et_system_prompt = g_string_new (NULL);
  if (args->et_module != NULL)
    {
      guint len = g_strv_length (args->et_module);
      et_modules = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                          (GDestroyNotify)et_module_data_free);

      for (guint k = 0; k < len; k++)
        {
          ETModuleData *md = et_module_data_new (
              args->et_module[k], args->et_module_param, &error);
          if (md == NULL)
            {
              g_warning ("Failed to load External Tool Module: %s\n%s\n",
                         args->et_module[k], error->message);
              g_error_free (error);
              error = NULL;
              continue;
            }

          if (!g_hash_table_replace (et_modules, g_strdup (md->et->name), md))
            g_warning ("Name duplication found: external tool: %s\n",
                       md->et->name);
        }

      et_system_prompt = generate_et_system_prompt (et_modules);
    }
  else
    g_string_printf (et_system_prompt, "No external tools are available.\n");

  g_string_replace (system_prompt, "{et_system_prompt}", et_system_prompt->str,
                    0);

  if (system_prompt->len > 0)
    {
      GString *system_prompt_template
          = lm_module->lm->chat_template ("System", system_prompt->str, NULL);
      if (!lm_module->lm->prompt (lm_module->lm, system_prompt_template,
                                  &error))
        {
          g_string_free (system_prompt_template, TRUE);
          goto on_error;
        }

      g_string_free (system_prompt_template, TRUE);
    }

  /* main loop */
  while (TRUE)
    {
      GString *user_input = NULL;
      GString *chat_template = NULL;
      GString *output = NULL;
      GString *command_output = NULL;
      gboolean thinking = FALSE;

      printf ("\nUser: ");
      fflush (stdout);
      user_input = read_user_input ();
      if (!strcmp (user_input->str, "!exit"))
        {
          g_string_free (user_input, TRUE);
          break;
        }
      if (!strncmp ("!think", user_input->str, 6))
        thinking = TRUE;

      do
        {
          if (user_input)
            chat_template = lm_module->lm->chat_template (
                "User", user_input->str, "Assistant", NULL);
          else
            {
              chat_template = lm_module->lm->chat_template (
                  "System", command_output->str, "Assistant", NULL);
              g_string_free (command_output, TRUE);
              command_output = NULL;
            }

          if (chat_history_stream
              && (g_output_stream_write (G_OUTPUT_STREAM (chat_history_stream),
                                         chat_template->str,
                                         chat_template->len, NULL, &error)
                  == -1))
            goto on_main_loop_error;

          printf ("\nAssistant: ");
          fflush (stdout);
          if (thinking && args->show_thinking)
            output = lm_module->lm->inference_with_think (
                lm_module->lm, chat_template, NULL, callback, callback, NULL,
                &error);
          else if (thinking)
            output = lm_module->lm->inference_with_think (
                lm_module->lm, chat_template, NULL, NULL, callback, NULL,
                &error);
          else
            output = lm_module->lm->inference (lm_module->lm, chat_template,
                                               callback, NULL, &error);
          if (output == NULL)
            goto on_main_loop_error;

          if (chat_history_stream
              && (g_output_stream_write (G_OUTPUT_STREAM (chat_history_stream),
                                         output->str, output->len, NULL,
                                         &error)
                  == -1))
            goto on_main_loop_error;

          command_output = process_command (output, et_modules);
          if (command_output->len > 0)
            printf ("\nSystem: %s", command_output->str);

          g_string_free (chat_template, TRUE);
          chat_template = NULL;
          g_string_free (user_input, TRUE);
          user_input = NULL;
        }
      while (command_output->len > 0);

      g_string_free (output, TRUE);
      continue;
    on_main_loop_error:
      g_string_free (command_output, TRUE);
      g_string_free (output, TRUE);
      g_string_free (chat_template, TRUE);
      g_string_free (user_input, TRUE);
      goto on_error;
    }

  if (chat_history_stream)
    g_object_unref (chat_history_stream);
  if (chat_history_file)
    g_object_unref (chat_history_file);
  if (et_modules)
    g_hash_table_unref (et_modules);
  if (et_system_prompt)
    g_string_free (et_system_prompt, TRUE);
  g_string_free (system_prompt, TRUE);
  lm_module_data_free (lm_module);
  args_free (args);

  return 0;
on_error:
  if (error)
    {
      fprintf (stderr, "Fatal: %s\n", error->message);
      g_error_free (error);
    }

  if (chat_history_stream)
    g_object_unref (chat_history_stream);
  if (chat_history_file)
    g_object_unref (chat_history_file);
  if (et_modules)
    g_hash_table_unref (et_modules);
  if (et_system_prompt)
    g_string_free (et_system_prompt, TRUE);
  g_string_free (system_prompt, TRUE);
  lm_module_data_free (lm_module);
  args_free (args);
  return 1;
}
