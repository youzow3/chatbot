/*   This file is part of Chatbot.
 *
 *  Chatbot is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or any later version.
 *
 *  Chatbot is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 *  You should have received a copy of the GNU General Public License along
 * with Chatbot. If not, see <https://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <stdio.h>

#include <gio/gio.h>
#include <gmodule.h>

#include <chatbot.h>

typedef struct
{
  ChatbotToolManager *tool_manager;
  gboolean is_firstline;
  gboolean is_command;
  gboolean is_command_running;
  GString *command;
} GeneratingData;

enum
{
  ARG_LANGUAGE_MODEL,
  ARG_LANGUAGE_MODEL_ARGS,
  ARG_TOOL,
  ARG_SYSTEM_PROMPT,
  ARG_SYSTEM_PROMPT_FILE,
  ARG_STATE_FILE,
  ARG_NULL,
  N_ARGS
};

static gchar *language_model_module_path = NULL;
static gchar *language_model_module_parameters = NULL;
static GStrv tool_module_paths = NULL;
static gchar *system_prompt = NULL;
static gchar *system_prompt_file = NULL;
static gchar *state_file = NULL;

static const GOptionEntry option_entries[N_ARGS] = {
  { "lm", 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_FILENAME,
    &language_model_module_path, "Shared library for language model.", "lib" },
  { "lm-args", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,
    &language_model_module_parameters, "Arguments for language model.",
    "args" },
  { "tool", 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_FILENAME_ARRAY,
    &tool_module_paths, "Shared library for tool.", "lib" },
  { "system-prompt", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING,
    &system_prompt, "System prompt for session." },
  { "system-prompt-file", 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_FILENAME,
    &system_prompt_file, "System prompt for session." },
  { "state-file", 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_FILENAME,
    &state_file, "State file for session." },
  G_OPTION_ENTRY_NULL
};

static gboolean
generating (ChatbotLanguageModel *lm, const gchar *text, gpointer user_data)
{
  GeneratingData *data = user_data;
  GError *error = NULL;

  if (data->is_firstline)
    {
      size_t len = strlen (text);
      data->is_firstline = FALSE;
      if (len == 0)
        return TRUE;

      if (text[0] == '!')
        {
          data->is_command = TRUE;
          data->command = g_string_new (text + 1);
          return TRUE;
        }
    }

  if (data->is_command)
    {
      for (const gchar *t = text; *t != 0; t++)
        if (*t == '\n')
          {
            g_string_append_len (data->command, text, t - text);
            data->is_command = FALSE;
          }

      if (data->is_command)
        g_string_append (data->command, text);
      else
        {
          int argc;
          GStrv argv;
          gchar *command = g_string_free_and_steal (data->command);
          data->command = NULL;

          if (!g_shell_parse_argv (command, &argc, &argv, &error))
            {
              GString *msg = g_string_new (NULL);
              g_string_printf (msg, "Failed to call command \"%s\": %s\n",
                               command, error->message);
              if (!chatbot_language_model_prefill (lm, msg->str, NULL))
                return FALSE;
              g_string_free (msg, TRUE);
            }

          data->is_command_running = TRUE;
          chatbot_tool_call (CHATBOT_TOOL (data->tool_manager), argc, argv);
          data->is_command_running = FALSE;

          g_strfreev (argv);
          g_free (command);
        }

      return TRUE;
    }

  if (text[strlen (text) - 1] == '\n')
    data->is_firstline = TRUE;

  if (!data->is_command_running)
    {
      printf ("%s", text);
      fflush (stdout);
    }
  return TRUE;
}

static gchar *
tool_mgr_stdin (ChatbotTool *tool, gpointer user_data)
{
  ChatbotLanguageModel *lm = CHATBOT_LANGUAGE_MODEL (user_data);
  gchar *generated;

  generated = chatbot_language_model_generate (lm, NULL);
  if (generated == NULL)
    generated = g_strdup ("");
  return generated;
}

static void
tool_mgr_stdout (ChatbotTool *tool, const gchar *text, gpointer user_data)
{
  ChatbotLanguageModel *lm = CHATBOT_LANGUAGE_MODEL (user_data);
  const gchar *rm[3] = { "System", NULL, NULL };
  gchar *rm_text;
  rm[1] = text;

  rm_text = chatbot_language_model_apply_chat_template (lm, (GStrv)rm);
  chatbot_language_model_prefill (lm, rm_text, NULL);
  g_free (rm_text);
}

static void
tool_mgr_stderr (ChatbotTool *tool, const gchar *text, gpointer user_data)
{
  ChatbotLanguageModel *lm = CHATBOT_LANGUAGE_MODEL (user_data);
  const gchar *rm[3] = { "System", NULL, NULL };
  gchar *rm_text;
  rm[1] = text;

  rm_text = chatbot_language_model_apply_chat_template (lm, (GStrv)rm);
  chatbot_language_model_prefill (lm, rm_text, NULL);
  g_free (rm_text);
}

static gchar *
read_user_input (void)
{
  const gsize chunk = 256;
  gsize offset = 0;
  gsize size = chunk;
  gchar *text = g_malloc (chunk);

  while (fgets (text + offset, chunk, stdin))
    {
      if ((strlen (text + offset) != chunk)
          || iscntrl (text[offset + chunk - 1]))
        break;
      text = g_realloc (text, size + chunk);
      offset += chunk;
      size += chunk;
    }

  return text;
}

int
main (int argc, char **argv)
{
  GOptionContext *option_context = NULL;
  GModule *language_model_module = NULL;
  GType (*language_model_module_get_type) (void);
  GArray *tool_modules = NULL;
  ChatbotLanguageModel *language_model = NULL;
  ChatbotToolManager *tool_manager = NULL;
  gboolean state_loaded = FALSE;

  GeneratingData generating_data;

  gchar *pending_system_prompt = NULL;
  gchar *pending_user_prompt = NULL;

  int ret_code = 1;
  GError *error = NULL;

  option_context = g_option_context_new (NULL);
  g_option_context_add_main_entries (option_context, option_entries, NULL);
  if (!g_option_context_parse (option_context, &argc, &argv, &error))
    goto on_error;

  if (language_model_module_path == NULL)
    {
      fprintf (stderr,
               "Fatal Error: Language model module path is not specified.\n");
      goto cleanup;
    }

  language_model_module = g_module_open_full (language_model_module_path,
                                              G_MODULE_BIND_LAZY, &error);
  if (language_model_module == NULL)
    goto cleanup;

  if (!g_module_symbol (language_model_module,
                        "language_model_module_get_type",
                        (gpointer *)&language_model_module_get_type))
    {
      fprintf (
          stderr,
          "Fatal Error: language_model_module_get_type is not found in %s\n",
          language_model_module_path);
      goto cleanup;
    }

  tool_modules = g_array_new (FALSE, FALSE, sizeof (GModule *));
  g_array_set_clear_func (tool_modules, (GDestroyNotify)g_module_close);
  for (gchar **iter = tool_module_paths; tool_module_paths && *iter; iter++)
    {
      GModule *tool_module
          = g_module_open_full (*iter, G_MODULE_BIND_LAZY, &error);
      if (tool_module == NULL)
        goto cleanup;
      g_array_append_val (tool_modules, tool_module);
    }

  language_model
      = chatbot_language_model_new (language_model_module_get_type (),
                                    language_model_module_parameters, &error);
  if (language_model == NULL)
    goto cleanup;

  tool_manager = chatbot_tool_manager_new ();
  for (guint i = 0; i < tool_modules->len; i++)
    {
      GModule *tool_module;
      GType (*tool_module_get_type) (void);
      ChatbotTool *tool;

      tool_module = g_array_index (tool_modules, GModule *, i);
      if (!g_module_symbol (tool_module, "tool_module_get_type",
                            (gpointer *)&tool_module_get_type))
        {
          fprintf (stderr,
                   "Fatal Error: tool_module_get_type is not found in %s\n",
                   tool_module_paths[i]);
          goto cleanup;
        }
      tool = chatbot_tool_new (tool_module_get_type ());
      chatbot_tool_manager_add_tool (tool_manager, tool);
      g_object_unref (tool);
    }

  if (state_file)
    {
      state_loaded = chatbot_language_model_load_state (language_model,
                                                        state_file, NULL);
      if (!state_loaded)
        fprintf (stderr, "State file is specified, but failed to load. "
                         "Continuing with default state.\n");
    }

  generating_data.tool_manager = tool_manager;
  g_signal_connect (language_model, "generating", G_CALLBACK (generating),
                    &generating_data);
  g_signal_connect (CHATBOT_TOOL (tool_manager), "stdin",
                    G_CALLBACK (tool_mgr_stdin), language_model);
  g_signal_connect (CHATBOT_TOOL (tool_manager), "stdout",
                    G_CALLBACK (tool_mgr_stdout), language_model);
  g_signal_connect (CHATBOT_TOOL (tool_manager), "stderr",
                    G_CALLBACK (tool_mgr_stderr), language_model);

  if (system_prompt_file)
    {
      g_clear_pointer (&system_prompt, g_free);
      if (!g_file_get_contents (system_prompt_file, &system_prompt, NULL,
                                &error))
        goto cleanup;
    }

  if (system_prompt && !state_loaded)
    pending_system_prompt = g_strdup (system_prompt);

  // Main Loop
  while (TRUE)
    {
      GStrv role_and_messages = NULL;
      gchar *chat_template = NULL;
      gchar *generated = NULL;
      GStrvBuilder *builder = g_strv_builder_new ();

      if (pending_system_prompt)
        g_strv_builder_add_many (builder, "System", pending_system_prompt,
                                 NULL);

      pending_user_prompt = read_user_input ();
      g_strv_builder_add_many (builder, "User", pending_user_prompt,
                               "Assistant", NULL);

      role_and_messages = g_strv_builder_end (builder);
      chat_template = chatbot_language_model_apply_chat_template (
          language_model, role_and_messages);

      if (!chatbot_language_model_prefill (language_model, chat_template,
                                           &error))
        goto loop_cleanup;

      generated = chatbot_language_model_generate (language_model, &error);
      if (generated == NULL)
        goto loop_cleanup;

    loop_cleanup:
      g_free (generated);
      g_clear_pointer (&pending_user_prompt, free);
      g_clear_pointer (&pending_system_prompt, g_free);
      g_strfreev (role_and_messages);
      g_free (chat_template);
      g_strv_builder_unref (builder);

      if (error)
        goto cleanup;
    }

  if (state_file
      && !chatbot_language_model_save_state (language_model, state_file,
                                             &error))
    goto cleanup;

  ret_code = 0;
cleanup:
  g_object_unref (tool_manager);
  g_object_unref (language_model);
  g_array_unref (tool_modules);
  g_module_close (language_model_module);
  g_free (state_file);
  g_free (system_prompt_file);
  g_free (system_prompt);
  g_strfreev (tool_module_paths);
  g_free (language_model_module_parameters);
  g_free (language_model_module_path);
  g_option_context_free (option_context);
  if (error)
    goto on_error;
  return ret_code;
on_error:
  fprintf (stderr, "Fatal Error: %s\n", error->message);
  g_error_free (error);
  return 1;
}
