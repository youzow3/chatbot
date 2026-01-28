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

G_BEGIN_DECLS

#define CHATBOT_TYPE_TOOL chatbot_tool_get_type ()
G_DECLARE_INTERFACE (ChatbotTool, chatbot_tool, CHATBOT, TOOL, GObject);

struct _ChatbotToolInterface
{
  GTypeInterface iface;
  const gchar *(*get_name) (ChatbotTool *tool);
  const gchar *(*get_description) (ChatbotTool *tool);
  const GStrv (*get_commands) (ChatbotTool *tool);
  int (*call) (ChatbotTool *tool, int argc, char **argv);
};

gpointer chatbot_tool_new (GType type);
const gchar *chatbot_tool_get_name (ChatbotTool *tool);
const gchar *chatbot_tool_get_description (ChatbotTool *tool);
const GStrv chatbot_tool_get_commands (ChatbotTool *tool);
int chatbot_tool_call (ChatbotTool *tool, int argc, char **argv);

G_END_DECLS
