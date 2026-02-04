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
 * ChatbotTool:
 *
 * External tool interface for Chatbot.
 *
 * Implementing all methods is required.
 */

#include "chatbot-tool.h"

enum
{
  STDIN,
  STDOUT,
  STDERR,
  N_SIGNALS
};

int signals[N_SIGNALS];

G_DEFINE_INTERFACE (ChatbotTool, chatbot_tool, CHATBOT_TYPE_MODULE);

static void
chatbot_tool_default_init (ChatbotToolInterface *iface)
{
  /**
   * ChatbotTool::stdin:
   * @tool: tool instance
   *
   * Signals to read text from a language model.
   *
   * fgets() with stdin or scanf() do not work for language models because
   * [iface@LanguageModel] does not have access to stdin or stdout.
   *
   * Returns: Inputs from a language model.
   */
  signals[STDIN] = g_signal_new ("stdin", CHATBOT_TYPE_TOOL, G_SIGNAL_RUN_LAST,
                                 0, NULL, NULL, NULL, G_TYPE_STRING, 0);
  /**
   * ChatbotTool::stdout:
   * @tool: tool instance
   * @cotent: string to print
   *
   * Signals to write out text to a language model.
   *
   * fputs() with stdout or printf() do not work for language models because
   * [iface@LanguageModel] does not have access to stdin or stdout.
   */
  signals[STDOUT]
      = g_signal_new ("stdout", CHATBOT_TYPE_TOOL, G_SIGNAL_RUN_LAST, 0, NULL,
                      NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
  /**
   * ChatbotTool::stderr:
   * @tool: tool instance
   * @cotent: string to print
   *
   * Signals to write out text which might be error to a language model.
   *
   * fputs() with stderr or fprintf() with stderr do not work for language
   * models because [iface@LanguageModel] does not have access to stderr.
   */
  signals[STDERR]
      = g_signal_new ("stderr", CHATBOT_TYPE_TOOL, G_SIGNAL_RUN_LAST, 0, NULL,
                      NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
}

/**
 * chatbot_tool_new:
 *
 * Currently just wrapper of g_object_new without properties.
 */
gpointer
chatbot_tool_new (GType type)
{
  return g_object_new (type, NULL);
}

/**
 * chatbot_tool_get_name:
 *
 * Return value should be always same content.
 *
 * Returns: Tool name
 */
const gchar *
chatbot_tool_get_name (ChatbotTool *tool)
{
  ChatbotToolInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), NULL);
  iface = CHATBOT_TOOL_GET_IFACE (tool);
  g_return_val_if_fail (iface->get_name, NULL);
  return iface->get_name (tool);
}

/**
 * chatbot_tool_get_description:
 *
 * Return value should be always same content.
 *
 * Returns: tool description
 */
const gchar *
chatbot_tool_get_description (ChatbotTool *tool)
{
  ChatbotToolInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), NULL);
  iface = CHATBOT_TOOL_GET_IFACE (tool);
  g_return_val_if_fail (iface->get_description, NULL);
  return iface->get_description (tool);
}

/**
 * chatbot_tool_get_commands:
 *
 * Returned value should be valid for [method@Tool.call] of argv[0].
 *
 * Returns: (transfer none): valid commands for this tool.
 */
const GStrv
chatbot_tool_get_commands (ChatbotTool *tool)
{
  ChatbotToolInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), NULL);
  iface = CHATBOT_TOOL_GET_IFACE (tool);
  g_return_val_if_fail (iface->get_commands, NULL);
  return iface->get_commands (tool);
}

/**
 * chatbot_tool_call:
 * @argc: length of @argv
 * @argv: (array length=argc): arguments
 *
 * Run the tool.
 *
 * Run the tool specified with %argv[0]. argc and argv are same as main(argc,
 * argv). For standard input, output and error, you should use %stdin, %stdout,
 * and %stderr signal instead of standard functions like printf, and scanf.
 *
 * Returns: 0 if succeed, non-zero (usually 1) on failure.
 */
int
chatbot_tool_call (ChatbotTool *tool, int argc, char **argv)
{
  ChatbotToolInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), -1);
  iface = CHATBOT_TOOL_GET_IFACE (tool);
  g_return_val_if_fail (iface->call, -1);
  return iface->call (tool, argc, argv);
}
