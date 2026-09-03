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
#include <json-glib/json-glib.h>

#include <chatbot/chatbot-data.h>
#include <chatbot/chatbot-message-role.h>

G_BEGIN_DECLS

#define CHATBOT_TYPE_MESSAGE chatbot_message_get_type ()
G_DECLARE_FINAL_TYPE (ChatbotMessage, chatbot_message, CHATBOT, MESSAGE,
                      GObject);

ChatbotMessage *chatbot_message_new (ChatbotMessageRole role,
                                     ChatbotData *data);
ChatbotMessage *chatbot_message_new_from_variant (GVariant *variant);
ChatbotMessage *chatbot_message_new_from_json (JsonReader *reader);
gboolean chatbot_message_is_valid (ChatbotMessage *message, GError **error);
ChatbotMessageRole chatbot_message_get_role (ChatbotMessage *message);
const gchar *chatbot_message_get_mime_type (ChatbotMessage *message);
ChatbotData *chatbot_message_get_data (ChatbotMessage *message);
GVariant *chatbot_message_to_variant (ChatbotMessage *message);
void chatbot_message_to_json (ChatbotMessage *message, JsonBuilder *builder);

G_END_DECLS
