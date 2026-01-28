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

#include "chatbot-tool.h"

G_BEGIN_DECLS

#define CHATBOT_TYPE_TOOL_MANAGER chatbot_tool_manager_get_type ()
G_DECLARE_FINAL_TYPE (ChatbotToolManager, chatbot_tool_manager, CHATBOT,
                      TOOL_MANAGER, GObject);

ChatbotToolManager *chatbot_tool_manager_new (void);
gboolean chatbot_tool_manager_add_tool (ChatbotToolManager *tool_manager,
                                        ChatbotTool *tool);
gboolean
chatbot_tool_manager_remove_tool_by_name (ChatbotToolManager *tool_manager,
                                          const gchar *name);
ChatbotTool *
chatbot_tool_manager_get_tool_by_name (ChatbotToolManager *tool_manager,
                                       const gchar *name);

G_END_DECLS
