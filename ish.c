
#include <ctype.h>
#include <stdio.h>

#include "chatbot.h"

typedef struct _ETRealModule
{
  ETModule public;
  gsize max_output;
} ETRealModule;

static int
ish_main (ETModule *module, int argc, char **argv, GString *output)
{
  ETRealModule *rmodule = (ETRealModule *)module;
  GString *command = NULL;
  GStrvBuilder *cmd_strv_builder = NULL;
  gchar **command_argv = NULL;
  GSubprocess *subprocess = NULL;
  GInputStream *subprocess_output = NULL;

  GError *error = NULL;

  char *buf;
  size_t buf_len;

  command = g_string_new (NULL);
  cmd_strv_builder = g_strv_builder_new ();
  for (int i = 1; i < argc; i++)
    {
      g_string_append_printf (command, " %s", argv[i]);
      g_strv_builder_add (cmd_strv_builder, argv[i]);
    }
  command_argv = g_strv_builder_end (cmd_strv_builder);

  // TODO implement simple dialog system to the parent module
  while (TRUE)
    {
      char ch;
      printf ("ish: User prompt: Do you allow to run command \"%s\"? [y/N]: ",
              command->str);
      fflush (stdout);
      if (scanf ("%c", &ch) != 1)
        continue;

      if (tolower (ch) == 'y')
        break;

      g_string_append_printf (output,
                              "ish: Fatal: Operation was rejected by User.\n");
      goto on_error;
    }

  buf = g_malloc0 (rmodule->max_output * 2);
  buf_len = 0;

  subprocess = g_subprocess_newv ((const gchar *const *)command_argv,
                                  G_SUBPROCESS_FLAGS_STDOUT_PIPE
                                      | G_SUBPROCESS_FLAGS_STDERR_MERGE,
                                  &error);
  if (subprocess == NULL)
    goto on_error;

  subprocess_output = g_subprocess_get_stdout_pipe (subprocess);
  g_subprocess_wait (subprocess, NULL, NULL);

  while (TRUE)
    {
      gssize buf_read
          = g_input_stream_read (subprocess_output, buf + rmodule->max_output,
                                 rmodule->max_output, NULL, &error);
      if (buf_read == -1)
        goto on_error;
      if (buf_read == 0)
        break;

      memcpy (buf, buf + rmodule->max_output, rmodule->max_output);
      memset (buf + rmodule->max_output, 0, rmodule->max_output);
    }

  buf_len = strlen (buf);
  if (buf_len > rmodule->max_output)
    g_string_append_printf (output,
                            "%s\nish: WARNING: Output length exceed limit "
                            "%zd, so some leading output are discarded.\n",
                            buf + (buf_len - rmodule->max_output),
                            rmodule->max_output);
  else
    g_string_append_printf (output, "%s\n", buf);

  g_string_append_printf (output,
                          "ish: subprocess finished with exit code %d\n",
                          g_subprocess_get_exit_status (subprocess));

  g_object_unref (subprocess);
  g_strfreev (command_argv);
  g_strv_builder_unref (cmd_strv_builder);
  g_string_free (command, TRUE);
  return 0;
on_error:
  if (error)
    {
      g_string_append_printf (output,
                              "ish: error while executing subprocess: %s\n",
                              error->message);
      g_error_free (error);
    }
  g_object_unref (subprocess_output);
  g_object_unref (subprocess);
  g_strfreev (command_argv);
  g_strv_builder_unref (cmd_strv_builder);
  g_string_free (command, TRUE);
  return 1;
}

char *command_names[] = { "ish" };

ETModuleCommand commands[] = {
  { "ish [commandline] - executing commands\n"
    "ish execute given command like normal shell via GSubprocess in GIO.\n"
    "because of runtime restriction, interactive applicatons cannot be used.\n"
    "For example, if you want to do file editing, you need to use sed instead "
    "of vim or emacs\n"
    "NOTE: ish just execute the specified command, so you can use \"[command] "
    "--help\" or something to see help.\n",
    ish_main }
};

static void
dispose (ETModule *module)
{
  ETRealModule *rmodule = (ETRealModule *)module;
  g_hash_table_unref (rmodule->public.commands);
  g_free (rmodule);
}

ETRealModule *
et_module_init (ETModuleParam *param, GError **error)
{
  ETRealModule *module;

  module = g_new (ETRealModule, 1);
  module->public.name = "ish";
  module->public.description = "Shell environment";
  module->public.commands = g_hash_table_new (g_str_hash, g_str_equal);
  g_hash_table_replace (module->public.commands, command_names[0],
                        &commands[0]);
  module->public.dispose = dispose;

  module->max_output = 4096;

  return module;
}
