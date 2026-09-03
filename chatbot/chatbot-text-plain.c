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
 * ChatbotTextPlain:
 *
 * `text/plain` implementation for [iface@Chatbot.Data].
 */

#include <chatbot/chatbot-error.h>
#include <chatbot/chatbot-text-plain.h>

struct _ChatbotTextPlain
{
  GObject parent;
  gchar *text;
};

enum
{
  PROP_TEXT = 1,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = {
  NULL,
};

static void chatbot_text_plain_data_iface_init (ChatbotDataInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (
    ChatbotTextPlain, chatbot_text_plain, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (CHATBOT_TYPE_DATA,
                           chatbot_text_plain_data_iface_init));

static gboolean
chatbot_text_plain_init_from_data (ChatbotData *data, gconstpointer raw_data,
                                   gsize size, GError **error)
{
  ChatbotTextPlain *text_plain;
  const gchar *maybe_text;

  g_return_val_if_fail (CHATBOT_IS_TEXT_PLAIN (data), FALSE);
  g_return_val_if_fail (raw_data != NULL, FALSE);
  g_return_val_if_fail (size > 0, FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);
  text_plain = CHATBOT_TEXT_PLAIN (data);

  maybe_text = raw_data;
  if (maybe_text[size - 1] != 0)
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Given data is not NULL terminated string.");
      return FALSE;
    }
  else if (strlen (maybe_text) != (size - 1))
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Given data has not expected length: %ld, expected: %ld.",
                   strlen (maybe_text), size - 1);
      return FALSE;
    }

  text_plain->text = g_strdup (raw_data);
  return TRUE;
}

static gboolean
chatbot_text_plain_init_from_text (ChatbotData *data, const gchar *text,
                                   GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_TEXT_PLAIN (data), FALSE);
  g_return_val_if_fail (text != NULL, FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  CHATBOT_TEXT_PLAIN (data)->text = g_strdup (text);

  return TRUE;
}

static gconstpointer
chatbot_text_plain_get_data (ChatbotData *data, gsize *size)
{
  ChatbotTextPlain *text_plain;

  g_return_val_if_fail (CHATBOT_IS_TEXT_PLAIN (data), NULL);
  text_plain = CHATBOT_TEXT_PLAIN (data);
  if (size)
    *size = strlen (text_plain->text) + 1;
  return text_plain->text;
}

static void
chatbot_text_plain_data_iface_init (ChatbotDataInterface *iface)
{
  iface->init_from_data = chatbot_text_plain_init_from_data;
  iface->init_from_text = chatbot_text_plain_init_from_text;
  iface->get_data = chatbot_text_plain_get_data;
  // Same as chatbot_text_plain_get_text.
  iface->get_text
      = (const gchar *(*)(ChatbotData *))chatbot_text_plain_get_text;
}

static void
chatbot_text_plain_set_property (GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
  ChatbotTextPlain *text_plain;

  g_return_if_fail (CHATBOT_IS_TEXT_PLAIN (object));
  text_plain = CHATBOT_TEXT_PLAIN (object);

  switch (property_id)
    {
    case PROP_TEXT:
      text_plain->text = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
chatbot_text_plain_get_property (GObject *object, guint property_id,
                                 GValue *value, GParamSpec *pspec)
{
  ChatbotTextPlain *text_plain;

  g_return_if_fail (CHATBOT_IS_TEXT_PLAIN (object));
  text_plain = CHATBOT_TEXT_PLAIN (object);

  switch (property_id)
    {
    case PROP_TEXT:
      g_value_set_string (value, text_plain->text);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
chatbot_text_plain_finalize (GObject *object)
{
  ChatbotTextPlain *text_plain;

  g_return_if_fail (CHATBOT_IS_TEXT_PLAIN (object));
  text_plain = CHATBOT_TEXT_PLAIN (object);

  g_free (text_plain->text);

  G_OBJECT_CLASS (chatbot_text_plain_parent_class)->finalize (object);
}

static void
chatbot_text_plain_class_init (ChatbotTextPlainClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = chatbot_text_plain_set_property;
  object_class->get_property = chatbot_text_plain_get_property;
  object_class->finalize = chatbot_text_plain_finalize;

  /**
   * ChatbotTextPlain:text:
   *
   * Text data
   */
  properties[PROP_TEXT]
      = g_param_spec_string ("text", "text", "plain text data", "",
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
chatbot_text_plain_init (ChatbotTextPlain *plain_text)
{
}

/**
 * chatbot_text_plain_new:
 * @text: Text data
 *
 * Constructs `text/plain` data with given @text.
 *
 * Returns: Newly created instance.
 */
ChatbotTextPlain *
chatbot_text_plain_new (const gchar *text)
{
  g_return_val_if_fail (text != NULL, NULL);
  return g_object_new (CHATBOT_TYPE_TEXT_PLAIN, "text", text, NULL);
}

/**
 * chatbot_text_plain_get_text:
 * @text_plain: self
 *
 * Gets containing text data.
 *
 * This method returns same value as [method@Chatbot.Data.get_text].
 *
 * Returns: Containing text data.
 */
const gchar *
chatbot_text_plain_get_text (ChatbotTextPlain *text_plain)
{
  g_return_val_if_fail (CHATBOT_IS_TEXT_PLAIN (text_plain), NULL);
  return CHATBOT_TEXT_PLAIN (text_plain)->text;
}
