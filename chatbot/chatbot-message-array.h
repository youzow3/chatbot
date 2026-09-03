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

#include <chatbot/chatbot-message.h>

G_BEGIN_DECLS

#define CHATBOT_TYPE_MESSAGE_ARRAY chatbot_message_array_get_type ()
GType chatbot_message_array_get_type (void) G_GNUC_CONST;

typedef struct _ChatbotMessageArray ChatbotMessageArray;

ChatbotMessageArray *chatbot_message_array_new (void);
ChatbotMessageArray *chatbot_message_array_new_from_variant (GVariant *variant,
                                                             GError **error);
ChatbotMessageArray *chatbot_message_array_new_from_json (JsonReader *reader,
                                                          GError **error);
ChatbotMessageArray *
chatbot_message_array_copy (ChatbotMessageArray *message_array);
void chatbot_message_array_free (ChatbotMessageArray *message_array);
GVariant *
chatbot_message_array_to_variant (ChatbotMessageArray *message_array);
void chatbot_message_array_to_json (ChatbotMessageArray *message_array,
                                    JsonBuilder *builder);
ChatbotMessage *chatbot_message_array_at (ChatbotMessageArray *message_array,
                                          guint index);
gsize chatbot_message_array_length (ChatbotMessageArray *message_array);
void chatbot_message_array_push (ChatbotMessageArray *message_array,
                                 ChatbotMessage *message);
void chatbot_message_array_push_take (ChatbotMessageArray *message_array,
                                      ChatbotMessage *message);
ChatbotMessage *chatbot_message_array_pop (ChatbotMessageArray *message_array);

G_END_DECLS
