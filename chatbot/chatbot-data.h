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

#define CHATBOT_TYPE_DATA chatbot_data_get_type ()
G_DECLARE_INTERFACE (ChatbotData, chatbot_data, CHATBOT, DATA, GObject);

struct _ChatbotDataInterface
{
  GTypeInterface iface;
  const gchar *(*get_string) (ChatbotData *data);
  const GStrv (*get_strings) (ChatbotData *data);
};

const gchar *chatbot_data_get_string (ChatbotData *data);
const GStrv chatbot_data_get_strings (ChatbotData *data);

G_END_DECLS
