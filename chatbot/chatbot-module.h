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

#define CHATBOT_TYPE_MODULE chatbot_module_get_type ()
G_DECLARE_DERIVABLE_TYPE (ChatbotModule, chatbot_module, CHATBOT, MODULE,
                          GObject);

struct _ChatbotModuleClass
{
  GObjectClass parent_class;
};

ChatbotModule *chatbot_module_new (GType type, const gchar *parameter,
                                   GError **error);
const gchar *chatbot_module_get_name (ChatbotModule *module);

G_END_DECLS
