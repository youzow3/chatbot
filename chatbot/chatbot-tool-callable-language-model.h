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
#pragma once

#include <glib-object.h>

#include "chatbot-language-model.h"
#include "chatbot-tool.h"

G_BEGIN_DECLS

#define CHATBOT_TYPE_TOOL_CALLABLE_LANGUAGE_MODEL                             \
  chatbot_tool_callable_language_model_get_type ()
G_DECLARE_INTERFACE (ChatbotToolCallableLanguageModel,
                     chatbot_tool_callable_language_model, CHATBOT,
                     TOOL_CALLABLE_LANGUAGE_MODEL, ChatbotLanguageModel);

struct _ChatbotToolCallableLanguageModelInterface
{
  GTypeInterface iface;
  gboolean (*add_tool) (ChatbotToolCallableLanguageModel *language_model,
                        ChatbotTool *tool, GError **error);
  gboolean (*remove_tool) (ChatbotToolCallableLanguageModel *language_model,
                           const gchar *tool_name);
};

gboolean chatbot_tool_callable_language_model_add_tool (
    ChatbotToolCallableLanguageModel *language_model, ChatbotTool *tool,
    GError **error);
gboolean chatbot_tool_callable_language_model_remove_tool (
    ChatbotToolCallableLanguageModel *language_model, const gchar *tool_name);

G_END_DECLS
