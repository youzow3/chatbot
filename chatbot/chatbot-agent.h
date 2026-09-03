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

#include <chatbot/chatbot-message-array.h>
#include <chatbot/chatbot-module.h>
#include <chatbot/chatbot-tool.h>

G_BEGIN_DECLS

#define CHATBOT_TYPE_AGENT chatbot_agent_get_type ()
G_DECLARE_INTERFACE (ChatbotAgent, chatbot_agent, CHATBOT, AGENT,
                     ChatbotModule);

struct _ChatbotAgentInterface
{
  GTypeInterface iface;
  ChatbotMessageArray *(*act) (ChatbotAgent *agent,
                               ChatbotMessageArray *messages,
                               GCancellable *cancellable, GError **error);
  gboolean (*add_tool) (ChatbotAgent *agent, ChatbotTool *tool,
                        GError **error);
  gboolean (*remove_tool) (ChatbotAgent *agent, ChatbotTool *tool);
};

ChatbotMessageArray *chatbot_agent_act (ChatbotAgent *agent,
                                        ChatbotMessageArray *messages,
                                        GCancellable *cancellable,
                                        GError **error);
void chatbot_agent_act_async (ChatbotAgent *agent,
                              ChatbotMessageArray *messages,
                              GCancellable *cancellable,
                              GAsyncReadyCallback callback,
                              gpointer user_data);
ChatbotMessageArray *chatbot_agent_act_finish (ChatbotAgent *agent,
                                               GAsyncResult *result,
                                               GError **error);
gboolean chatbot_agent_add_tool (ChatbotAgent *agent, ChatbotTool *tool,
                                 GError **error);
gboolean chatbot_agent_remove_tool (ChatbotAgent *agent, ChatbotTool *tool);

G_END_DECLS
