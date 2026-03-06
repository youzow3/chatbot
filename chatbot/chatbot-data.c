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
 * ChatbotData:
 *
 * Interface to represent data for training. Currently string and string array
 * are supported.
 */

#include "chatbot-data.h"

G_DEFINE_INTERFACE (ChatbotData, chatbot_data, G_TYPE_OBJECT);

static void
chatbot_data_default_init (ChatbotDataInterface *iface)
{
}

/**
 * chatbot_data_get_string:
 *
 * Returns: string instance holds.
 */
const gchar *
chatbot_data_get_string (ChatbotData *data)
{
  ChatbotDataInterface *iface;
  g_return_val_if_fail (CHATBOT_IS_DATA (data), NULL);
  iface = CHATBOT_DATA_GET_IFACE (data);
  g_return_val_if_fail (iface->get_string, NULL);
  return iface->get_string (data);
}

/**
 * chatbot_data_get_strings:
 *
 * Returns: (transfer none): string array instance holds.
 */
const GStrv
chatbot_data_get_strings (ChatbotData *data)
{
  ChatbotDataInterface *iface;
  g_return_val_if_fail (CHATBOT_IS_DATA (data), NULL);
  iface = CHATBOT_DATA_GET_IFACE (data);
  g_return_val_if_fail (iface->get_strings, NULL);
  return iface->get_strings (data);
}
