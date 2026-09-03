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
 * Interface to represent data.
 *
 * All implementations are needed to be registered via
 * [func@Chatbot.Data.iface_register_type] with appropriate MIME type.
 */

#include <chatbot/chatbot-error.h>
#include <chatbot/chatbot-text-plain.h>

#include <chatbot/chatbot-data.h>

static GHashTable *mime_type_to_gtype = NULL; // key: str, value: GType *
static GHashTable *gtype_to_mime_type = NULL; // key: GType *, value: str

G_DEFINE_INTERFACE (ChatbotData, chatbot_data, G_TYPE_OBJECT);

static void
_register_builtin_types (void)
{
  static gboolean registered = FALSE;
  if (registered)
    return;
  registered = TRUE;

  chatbot_data_iface_register_type ("text/plain", CHATBOT_TYPE_TEXT_PLAIN);
}

static void
chatbot_data_default_init (ChatbotDataInterface *iface)
{
}

static guint
_type_hash (gconstpointer p)
{
  gint64 _p = *(GType *)p;
  return g_int64_hash (&_p);
}

static gboolean
_type_equal (gconstpointer a, gconstpointer b)
{
  return *(GType *)a == *(GType *)b;
}

/**
 * chatbot_data_iface_register_type:
 * @mime_type: MIME type
 * @type: A #GType
 *
 * Registers @type as [iface@Chatbot.Data] implementation for @mime_type data.
 */
void
chatbot_data_iface_register_type (const gchar *mime_type, GType type)
{
  char *_mime_type;
  GType *_type;

  g_return_if_fail (mime_type != NULL);
  g_return_if_fail (g_type_is_a (type, CHATBOT_TYPE_DATA));
  _register_builtin_types ();

  if ((mime_type_to_gtype == NULL) && (gtype_to_mime_type == NULL))
    {
      mime_type_to_gtype
          = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
      gtype_to_mime_type
          = g_hash_table_new_full (_type_hash, _type_equal, NULL, NULL);
    }

  g_return_if_fail ((mime_type_to_gtype != NULL)
                    && (gtype_to_mime_type != NULL));

  _mime_type = g_strdup (mime_type);
  _type = g_new (GType, 1);
  *_type = type;
  g_hash_table_insert (mime_type_to_gtype, _mime_type, _type);
  g_hash_table_insert (gtype_to_mime_type, _type, _mime_type);
}

/**
 * chatbot_data_iface_unregister_type:
 * @mime_type: MIME type
 *
 * Unregisters [iface@Chatbot.Data] interface for MIME type @mime_type.
 */
void
chatbot_data_iface_unregister_type (const gchar *mime_type)
{
  GType *type;
  g_return_if_fail (mime_type != NULL);

  type = g_hash_table_lookup (mime_type_to_gtype, mime_type);
  if (type == NULL)
    return;
  g_hash_table_remove (gtype_to_mime_type, type);
  g_hash_table_remove (mime_type_to_gtype, mime_type);
}

/**
 * chatbot_data_iface_mime_type_from_gtype:
 * @type: A #GType
 *
 * Gets MIME type string from given @type.
 *
 * Returns: (nullable): MIME type assciated with @type.
 */
const gchar *
chatbot_data_iface_mime_type_from_gtype (GType type)
{
  g_return_val_if_fail (g_type_is_a (type, CHATBOT_TYPE_DATA), NULL);

  if (gtype_to_mime_type == NULL)
    return NULL;
  return g_hash_table_lookup (gtype_to_mime_type, &type);
}

/**
 * chatbot_data_new_from_data:
 * @mime_type: MIME type.
 * @data: Pointer to raw data.
 * @size: Size of @raw_data.
 * @error: (out) (optional): Location to store error.
 *
 * Constructs an instance from given @data using registered
 * [iface@Chatbot.Data] implementation.
 *
 * To register interface implementation use
 * [func@Chatbot.Data.iface_register_type].
 *
 * Returns: (transfer full): Newly created instance from @data.
 */
gpointer
chatbot_data_new_from_data (const gchar *mime_type, gconstpointer data,
                            gsize size, GError **error)
{
  GType *type;
  ChatbotData *instance;
  ChatbotDataInterface *iface;

  g_return_val_if_fail (mime_type != NULL, NULL);
  g_return_val_if_fail (data != NULL, NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);
  _register_builtin_types ();

  type = mime_type_to_gtype != NULL
             ? g_hash_table_lookup (mime_type_to_gtype, mime_type)
             : NULL;
  if (type == NULL)
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_NOT_FOUND,
                   "ChatbotData implementation for \"%s\" was not found.",
                   mime_type);
      return NULL;
    }

  instance = g_object_new (*type, NULL);
  iface = CHATBOT_DATA_GET_IFACE (instance);
  g_return_val_if_fail (iface->init_from_data != NULL, NULL);
  if (!iface->init_from_data (instance, data, size, error))
    return NULL;
  return instance;
}

/**
 * chatbot_data_new_from_text:
 * @mime_type: MIME type
 * @text: data represented in text.
 * @error: (out) (optional): Location to store error.
 *
 * Constructs an instance by using UTF-8 text formed data.
 *
 * Returns: (transfer full): Newly constructed instance from @text_data.
 */
gpointer
chatbot_data_new_from_text (const gchar *mime_type, const gchar *text,
                            GError **error)
{
  GType *type;
  ChatbotData *instance;
  ChatbotDataInterface *iface;

  g_return_val_if_fail (mime_type != NULL, NULL);
  g_return_val_if_fail (text != NULL, NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);
  _register_builtin_types ();

  type = mime_type_to_gtype != NULL
             ? g_hash_table_lookup (mime_type_to_gtype, mime_type)
             : NULL;
  if (type == NULL)
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_NOT_FOUND,
                   "ChatbotData implementation for \"%s\" was not found.",
                   mime_type);
      return NULL;
    }

  instance = g_object_new (*type, NULL);
  iface = CHATBOT_DATA_GET_IFACE (instance);
  g_return_val_if_fail (iface->init_from_text != NULL, NULL);
  if (!iface->init_from_text (instance, text, error))
    return NULL;
  return instance;
}

/**
 * chatbot_data_get_data:
 * @size: (out) (optional): Location to store data size
 *
 * Gets raw data.
 *
 * Returns: Pointer to raw data
 */
gconstpointer
chatbot_data_get_data (ChatbotData *data, gsize *size)
{
  ChatbotDataInterface *klass;

  g_return_val_if_fail (CHATBOT_IS_DATA (data), NULL);
  klass = CHATBOT_DATA_GET_IFACE (data);
  g_return_val_if_fail (klass->get_data != NULL, NULL);
  return klass->get_data (data, size);
}

/**
 * chatbot_data_get_text:
 * @data: self
 *
 * Gets data in UTF-8 text form.
 *
 * It can be its text data or base64 encoded binary data.
 *
 * Returns: Text data representing @data.
 */
const gchar *
chatbot_data_get_text (ChatbotData *data)
{
  ChatbotDataInterface *klass;
  g_return_val_if_fail (CHATBOT_IS_DATA (data), NULL);
  klass = CHATBOT_DATA_GET_IFACE (data);
  g_return_val_if_fail (klass->get_text != NULL, NULL);
  return klass->get_text (data);
}
