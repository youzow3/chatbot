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
 * ChatbotToolCallableLanguageModel:
 *
 * Language model with tool calling support.
 *
 * Implementers are responsible for following things:
 *
 * 1. Supply appropriate prompt to describe tool usage via system role.
 *
 * 2. Make [method@ChatbotLanguageModel.generate] to handle tool calling.
 */

#include "chatbot-tool-callable-language-model.h"

G_DEFINE_INTERFACE (ChatbotToolCallableLanguageModel,
                    chatbot_tool_callable_language_model,
                    CHATBOT_TYPE_LANGUAGE_MODEL);

static void
chatbot_tool_callable_language_model_default_init (
    ChatbotToolCallableLanguageModelInterface *iface)
{
  /**
   * ChatbotToolCallableLanguageModel:tools:
   *
   * Tools that language model can use
   *
   * Tool could be built-in, or added by
   * [method@ChatbotToolCallableLanguageModel.add_tool].
   */
  g_object_interface_install_property (
      iface, g_param_spec_boxed ("tools", "tools", "External tools",
                                 G_TYPE_PTR_ARRAY, G_PARAM_READABLE));
}

/**
 * chatbot_tool_callable_language_model_add_tool:
 * @tool: tool to add
 * @error: (nullable): pointer to the #GError* to store the error
 *
 * Add tool to the language model
 *
 * Returns: TRUE if tool is added, and FALSE if it's not added.
 */
gboolean
chatbot_tool_callable_language_model_add_tool (
    ChatbotToolCallableLanguageModel *language_model, ChatbotTool *tool,
    GError **error)
{
  ChatbotToolCallableLanguageModelInterface *iface;

  g_return_val_if_fail (
      CHATBOT_IS_TOOL_CALLABLE_LANGUAGE_MODEL (language_model), FALSE);
  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);
  iface = CHATBOT_TOOL_CALLABLE_LANGUAGE_MODEL_GET_IFACE (language_model);
  g_return_val_if_fail (iface->add_tool != NULL, FALSE);
  return iface->add_tool (language_model, tool, error);
}

/**
 * chatbot_tool_callable_language_model_remove_tool:
 * @tool_name: tool name to remove
 *
 * Remove tool from the language model
 *
 * Returns: TRUE if tool is removed, and FALSE if it's not removed.
 */
gboolean
chatbot_tool_callable_language_model_remove_tool (
    ChatbotToolCallableLanguageModel *language_model, const gchar *tool_name)
{
  ChatbotToolCallableLanguageModelInterface *iface;

  g_return_val_if_fail (
      CHATBOT_IS_TOOL_CALLABLE_LANGUAGE_MODEL (language_model), FALSE);
  g_return_val_if_fail (tool_name != NULL, FALSE);
  iface = CHATBOT_TOOL_CALLABLE_LANGUAGE_MODEL_GET_IFACE (language_model);
  g_return_val_if_fail (iface->add_tool != NULL, FALSE);
  return iface->remove_tool (language_model, tool_name);
}
