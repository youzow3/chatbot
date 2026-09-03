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

#include <chatbot/chatbot-data.h>

#define CHATBOT_TYPE_TEXT_PLAIN chatbot_text_plain_get_type ()
G_DECLARE_FINAL_TYPE (ChatbotTextPlain, chatbot_text_plain, CHATBOT,
                      TEXT_PLAIN, GObject);

ChatbotTextPlain *chatbot_text_plain_new (const gchar *text);
const gchar *chatbot_text_plain_get_text(ChatbotTextPlain *text_plain);
