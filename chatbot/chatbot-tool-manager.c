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

/**
 * ChatbotToolManager:
 *
 * External tool manager for Chatbot.
 *
 * This class implement [iface@Tool], but it doesn't implement [method@Tool.get_name] and
 * [method@Tool.get_description] because this class is not intended to be used
 * as a Tool. Programmer should consider this class as helper class for
 * multiple tools.
 */

#include "chatbot-tool-manager.h"

enum
{
  STDIN,
  STDOUT,
  STDERR,
  N_SIGNALS
};

static guint signals[N_SIGNALS];

struct _ChatbotToolManager
{
  GObject parent_instance;
  GHashTable *name_tool; // <gchar*, ChatbotTool> (tool name, ChatbotTool*)
  GHashTable
      *command_name; // <gchar*, GArray*> (command name, tool name array)
};

static void chatbot_tool_manager_tool_init (ChatbotToolInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (
    ChatbotToolManager, chatbot_tool_manager, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (CHATBOT_TYPE_TOOL, chatbot_tool_manager_tool_init));

static int
chatbot_tool_manager_call (ChatbotTool *tool, int argc, char **argv)
{
  GArray *cmd_tool;
  const gchar *cmd = argv[0];
  ChatbotToolManager *tool_manager = CHATBOT_TOOL_MANAGER (tool);

  cmd_tool = g_hash_table_lookup (tool_manager->command_name, cmd);
  if (cmd_tool == NULL) // No tool provide specified command
    {
      gchar *msg
          = g_strdup_printf ("No command named \"%s\" is found.\n", cmd);
      g_signal_emit_by_name (CHATBOT_TOOL (cmd_tool), "stderr", msg);
      g_free (msg);
      return -1;
    }

  if (cmd_tool->len > 1) // If some tools provide command
    {
      GString *msg = g_string_new (NULL);
      g_string_printf (msg, "Command \"%s\" is provided by some tools:", cmd);
      for (guint i = 0; i < cmd_tool->len; i++)
        g_string_append_printf (msg, " %s%c", cmd,
                                (i + 1) == cmd_tool->len ? '\n' : ',');
      g_string_append_printf (msg,
                              "To call this command, you need to specify tool "
                              "name, like \"[namespace]:%s\".\n",
                              cmd);
      g_signal_emit_by_name (CHATBOT_TOOL (cmd_tool), "stderr", msg->str);
      g_string_free (msg, TRUE);
      return -1;
    }

  return chatbot_tool_call (
      g_hash_table_lookup (tool_manager->name_tool,
                           g_array_index (cmd_tool, const gchar *, 0)),
      argc, argv);
}

static gchar *
chatbot_tool_manager_stdin (ChatbotTool *tool, gpointer user_data)
{
  gchar *ret;
  g_signal_emit (CHATBOT_TOOL_MANAGER (user_data), signals[STDIN], 0, &ret);
  return ret;
}

static void
chatbot_tool_manager_stdout (ChatbotTool *tool, const gchar *text,
                             gpointer user_data)
{
  g_signal_emit (CHATBOT_TOOL_MANAGER (user_data), signals[STDOUT], 0, text);
}

static void
chatbot_tool_manager_stderr (ChatbotTool *tool, const gchar *text,
                             gpointer user_data)
{
  g_signal_emit (CHATBOT_TOOL_MANAGER (user_data), signals[STDERR], 0, text);
}

static void
chatbot_tool_manager_tool_init (ChatbotToolInterface *iface)
{
  iface->call = chatbot_tool_manager_call;

  signals[STDIN] = g_signal_lookup ("stdin", CHATBOT_TYPE_TOOL);
  signals[STDOUT] = g_signal_lookup ("stdout", CHATBOT_TYPE_TOOL);
  signals[STDERR] = g_signal_lookup ("stderr", CHATBOT_TYPE_TOOL);
}

static void
chatbot_tool_manager_dispose__name_tool (gpointer key, gpointer value,
                                         gpointer user_data)
{
  ChatbotTool *tool = CHATBOT_TOOL (value);
  ChatbotToolManager *tool_manager = CHATBOT_TOOL_MANAGER (user_data);

  g_signal_handlers_disconnect_by_func (tool, chatbot_tool_manager_stdout,
                                        tool_manager);
  g_signal_handlers_disconnect_by_func (tool, chatbot_tool_manager_stderr,
                                        tool_manager);
  g_signal_handlers_disconnect_by_func (tool, chatbot_tool_manager_stdin,
                                        tool_manager);
}

static void
chatbot_tool_manager_dispose (GObject *object)
{
  ChatbotToolManager *tool_manager = CHATBOT_TOOL_MANAGER (object);

  g_clear_pointer (&tool_manager->command_name, g_hash_table_unref);
  g_hash_table_foreach (tool_manager->name_tool,
                        chatbot_tool_manager_dispose__name_tool, object);
  g_clear_pointer (&tool_manager->name_tool, g_hash_table_unref);

  G_OBJECT_CLASS (chatbot_tool_manager_parent_class)->dispose (object);
}

static void
chatbot_tool_manager_class_init (ChatbotToolManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = chatbot_tool_manager_dispose;
}

static void
chatbot_tool_manager_init (ChatbotToolManager *tool_manager)
{
  tool_manager->name_tool = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                   g_free, g_object_unref);
  tool_manager->command_name = g_hash_table_new_full (
      g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_array_unref);
}

ChatbotToolManager *
chatbot_tool_manager_new (void)
{
  return g_object_new (CHATBOT_TYPE_TOOL_MANAGER, NULL);
}

/**
 * chatbot_tool_manager_add_tool:
 * @tool: tool
 *
 * Add tool to manager.
 *
 * If name conflict is occured, this function fails.
 *
 * Returns: %TRUE if successfully added, %FALSE on failure.
 */
gboolean
chatbot_tool_manager_add_tool (ChatbotToolManager *tool_manager,
                               ChatbotTool *tool)
{
  const gchar *name;
  GStrv commands;

  g_return_val_if_fail (CHATBOT_IS_TOOL_MANAGER (tool_manager), FALSE);
  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), FALSE);

  name = chatbot_tool_get_name (tool);
  g_return_val_if_fail (name, FALSE);

  if (!g_hash_table_insert (tool_manager->name_tool, g_strdup (name), tool))
    return FALSE;

  commands = chatbot_tool_get_commands (tool);
  for (gchar **iter = commands; *iter; iter++)
    {
      GArray *array = g_hash_table_lookup (tool_manager->command_name, *iter);
      if (array != NULL)
        {
          g_array_append_val (array, tool);
          continue;
        }

      array = g_array_new (FALSE, FALSE, sizeof (ChatbotTool *));
      g_array_set_clear_func (array, g_free);
      g_object_ref (tool);
      g_signal_connect (tool, "stdin", G_CALLBACK (chatbot_tool_manager_stdin),
                        tool_manager);
      g_signal_connect (tool, "stdout",
                        G_CALLBACK (chatbot_tool_manager_stdout),
                        tool_manager);
      g_signal_connect (tool, "stderr",
                        G_CALLBACK (chatbot_tool_manager_stderr),
                        tool_manager);
      g_array_append_val (array, tool);
      g_hash_table_insert (tool_manager->command_name, g_strdup (*iter),
                           array);
    }

  return TRUE;
}

/**
 * chatbot_tool_manager_remove_tool_by_name:
 * @name: tool name
 *
 * Try to remove tool by name.
 *
 * If name is not found, this function fails.
 *
 * Returns: %TRUE if tool is removed, %FALSE if not.
 */
gboolean
chatbot_tool_manager_remove_tool_by_name (ChatbotToolManager *tool_manager,
                                          const gchar *name)
{
  ChatbotTool *tool;
  GStrv commands;

  g_return_val_if_fail (CHATBOT_IS_TOOL_MANAGER (tool_manager), FALSE);
  g_return_val_if_fail (name, FALSE);

  tool = g_hash_table_lookup (tool_manager->name_tool, name);
  if (tool == NULL)
    return FALSE;

  commands = chatbot_tool_get_commands (tool);
  for (gchar **iter = commands; *iter; iter++)
    {
      guint idx;
      GArray *array = g_hash_table_lookup (tool_manager->command_name, *iter);
      if (array->len == 1)
        {
          g_hash_table_remove (tool_manager->command_name, *iter);
          continue;
        }

      g_array_sort (array, (GCompareFunc)strcmp);
      g_array_binary_search (array, *iter, (GCompareFunc)strcmp, &idx);
      g_array_remove_index (array, idx);
    }

  g_signal_handlers_disconnect_by_func (
      tool, G_CALLBACK (chatbot_tool_manager_stderr), tool_manager);
  g_signal_handlers_disconnect_by_func (
      tool, G_CALLBACK (chatbot_tool_manager_stdout), tool_manager);
  g_signal_handlers_disconnect_by_func (
      tool, G_CALLBACK (chatbot_tool_manager_stdin), tool_manager);

  g_hash_table_remove (tool_manager->name_tool, name);

  return TRUE;
}

/**
 * chatbot_tool_manager_get_tool_by_name:
 * @name: tool name
 *
 * Get tool instance by name.
 *
 * Returns: (nullable) (transfer none): tool instance if found, %NULL if not
 * found.
 */
ChatbotTool *
chatbot_tool_manager_get_tool_by_name (ChatbotToolManager *tool_manager,
                                       const gchar *name)
{
  g_return_val_if_fail (CHATBOT_IS_TOOL_MANAGER (tool_manager), NULL);
  g_return_val_if_fail (name, NULL);

  return g_hash_table_lookup (tool_manager->name_tool, name);
}
