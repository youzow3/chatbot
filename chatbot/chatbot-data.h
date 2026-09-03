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

G_BEGIN_DECLS

#define CHATBOT_TYPE_DATA chatbot_data_get_type ()
G_DECLARE_INTERFACE (ChatbotData, chatbot_data, CHATBOT, DATA, GObject);

struct _ChatbotDataInterface
{
  GTypeInterface iface;
  /**
   * ChatbotData::init_from_data:
   * @data: self
   * @raw_data: Pointer to the raw data.
   * @size: Size of @raw_data.
   * @error: location to store a runtime error.
   *
   * Initialize instance via raw data.
   *
   * Returns: %TRUE if successfully initialized, and %FALSE if failed to
   * initialize.
   */
  gboolean (*init_from_data) (ChatbotData *data, gconstpointer raw_data,
                              gsize size, GError **error);
  /**
   * ChatbotData::init_from_text:
   * @data: self
   * @text: Pointer to the text formed data.
   * @error: location to store a runtime error.
   *
   * Initialize instance via text formed data.
   *
   * Returns: %TRUE if successfully initialized, and %FALSE if failed to
   * initialize.
   */
  gboolean (*init_from_text) (ChatbotData *data, const gchar *text,
                              GError **error);
  gconstpointer (*get_data) (ChatbotData *data, gsize *size);
  const gchar *(*get_text) (ChatbotData *data);
};

void chatbot_data_iface_register_type (const gchar *mime_type, GType type);
void chatbot_data_iface_unregister_type (const gchar *mime_type);
const gchar *chatbot_data_iface_mime_type_from_gtype (GType type);

gpointer chatbot_data_new_from_data (const gchar *mime_type,
                                     gconstpointer data, gsize size,
                                     GError **error);
gpointer chatbot_data_new_from_text (const gchar *mime_type, const gchar *text,
                                     GError **error);
gconstpointer chatbot_data_get_data (ChatbotData *data, gsize *size);
const gchar *chatbot_data_get_text (ChatbotData *data);

G_END_DECLS
