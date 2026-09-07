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
 * ChatbotMessageArray:
 *
 * Helper structure to hold array of [class@Chatbot.Message].
 *
 * Internally, it uses FIFO buffer, thus, push/pop means append to last, and
 * remove (and take ownership) from first element.
 */

#include <json-glib/json-glib.h>

#include <chatbot/chatbot-error.h>
#include <chatbot/chatbot-message.h>

#include <chatbot/chatbot-message-array.h>

struct _ChatbotMessageArray
{
  GQueue *fifo;
};

G_DEFINE_BOXED_TYPE (ChatbotMessageArray, chatbot_message_array,
                     chatbot_message_array_copy, chatbot_message_array_free);

/**
 * chatbot_message_array_new:
 *
 * Creates message array.
 *
 * Returns: Newly allocated structure
 */
ChatbotMessageArray *
chatbot_message_array_new (void)
{
  ChatbotMessageArray *message_array = g_new (ChatbotMessageArray, 1);
  message_array->fifo = g_queue_new ();
  return message_array;
}

/**
 * chatbot_message_array_new_from_variant:
 * @variant: #GVariant
 * @error: (nullable): location to store error
 *
 * Creates message array from given @variant.
 *
 * Returns: Newly allocated structure loaded from @variant or %NULL on failure.
 */
ChatbotMessageArray *
chatbot_message_array_new_from_variant (GVariant *variant, GError **error)
{
  ChatbotMessageArray *message_array;
  GVariantIter *i;
  GVariant *element;

  g_return_val_if_fail (variant != NULL, NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);

  message_array = chatbot_message_array_new ();
  i = g_variant_iter_new (variant);
  while (g_variant_iter_next (i, "@a{sv}", &element))
    {
      ChatbotMessage *message;

      if (element == NULL)
        {
          g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                       "Failed to retrieve element from variant.");
          goto on_error;
        }

      message = chatbot_message_new_from_variant (element);
      if (!chatbot_message_is_valid (message, error))
        {
          g_object_unref (message);
          goto on_error;
        }
      chatbot_message_array_push (message_array, message);

      g_variant_unref (element);
    }
  g_variant_iter_free (i);

  return message_array;
on_error:
  chatbot_message_array_free (message_array);
  return NULL;
}

/**
 * chatbot_message_array_new_from_json:
 * @reader: JSON reader
 * @error: (nullable): location to store error
 *
 * Creates message array from JSON using given @reader.
 *
 * Returns: Newly allocated structure loaded from @reader or %NULL on failure.
 */
ChatbotMessageArray *
chatbot_message_array_new_from_json (JsonReader *reader, GError **error)
{
  ChatbotMessageArray *message_array;
  gint element_count;
  JsonReader *element_reader = NULL;

  g_return_val_if_fail (
      JSON_IS_READER (reader) && json_reader_is_object (reader), NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);
  message_array = chatbot_message_array_new ();

  if (json_reader_read_member (reader, "length"))
    element_count = json_reader_get_int_value (reader);
  else
    element_count = -1;
  json_reader_end_member (reader); // read_member("length")

  if (!json_reader_read_member (reader, "messages"))
    {
      if (error != NULL)
        *error = g_error_copy (json_reader_get_error (reader));
      json_reader_end_member (reader);
      goto on_error;
    }

  if (!json_reader_is_array (reader))
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "\"messages\" element must have array.");
      json_reader_end_member (reader); // read_member("messages")
      goto on_error;
    }

  if (element_count == -1)
    element_count = json_reader_count_elements (reader);
  element_reader = json_reader_new (json_reader_get_current_node (reader));
  for (gint i = 0; i < element_count; i++)
    {
      ChatbotMessage *message;

      if (!json_reader_read_element (element_reader, i))
        {
          if (error != NULL)
            *error = g_error_copy (json_reader_get_error (element_reader));
          json_reader_end_element (element_reader);
          json_reader_end_member (reader); // read_member("messages")
          goto on_error;
        }

      message = chatbot_message_new_from_json (element_reader);
      json_reader_end_element (element_reader); // read_element(i)

      if (!chatbot_message_is_valid (message, error))
        {
          g_object_unref (message);
          json_reader_end_member (reader); // read_member("messages")
          goto on_error;
        }

      chatbot_message_array_push_take (message_array, message);
    }

  json_reader_end_member (reader); // read_member("messages")
  g_object_unref (element_reader);
  return message_array;
on_error:
  if (element_reader)
    g_object_unref (element_reader);
  chatbot_message_array_free (message_array);
  return NULL;
}

static void
_fifo_ref (gpointer data, gpointer user_data)
{
  g_object_ref ((ChatbotMessage *)data);
}

/**
 * chatbot_message_array_copy:
 * @message_array: self
 *
 * Copies the given @message_array.
 *
 * The internal array is copied but its element stays same (with ref count
 * incremented), thus, changing the content(s) of elements could lead to
 * undefined behavior.
 *
 * Returns: Copied structure
 */
ChatbotMessageArray *
chatbot_message_array_copy (ChatbotMessageArray *message_array)
{
  ChatbotMessageArray *copy;
  g_return_val_if_fail (message_array != NULL, NULL);

  copy = g_new (ChatbotMessageArray, 1);
  copy->fifo = g_queue_copy (message_array->fifo);
  g_queue_foreach (copy->fifo, _fifo_ref, NULL);
  return copy;
}

/**
 * chatbot_message_array_free:
 * @message_array: self
 *
 * Frees up the structure
 */
void
chatbot_message_array_free (ChatbotMessageArray *message_array)
{
  g_return_if_fail (message_array != NULL);

  g_queue_free_full (message_array->fifo, g_object_unref);
  g_free (message_array);
}

static void
_to_variant (gpointer data, gpointer user_data)
{
  ChatbotMessage *message;
  GVariantBuilder *builder;

  g_return_if_fail (CHATBOT_IS_MESSAGE (data));
  g_return_if_fail (user_data != NULL);
  message = data;
  builder = user_data;

  g_variant_builder_add_value (builder, chatbot_message_to_variant (message));
}

/**
 * chatbot_message_array_to_variant:
 *
 * Converts to #GVariant.
 *
 * Returns: (transfer none): Floating, converted %GVariant.
 */
GVariant *
chatbot_message_array_to_variant (ChatbotMessageArray *message_array)
{
  GVariantBuilder builder;
  g_return_val_if_fail (message_array != NULL, NULL);

  g_variant_builder_init_static (&builder, G_VARIANT_TYPE ("aa{sv}"));
  g_queue_foreach (message_array->fifo, _to_variant, &builder);
  return g_variant_builder_end (&builder);
}

static void
_message_to_json (gpointer data, gpointer user_data)
{
  chatbot_message_to_json ((ChatbotMessage *)data, JSON_BUILDER (user_data));
}

/**
 * chatbot_message_array_to_json:
 * @message_array: self
 * @builder: JSON builder
 *
 * Converts to JSON and appends to @builder.
 *
 * It appends JSON object with element `length` and `messages`.
 */
void
chatbot_message_array_to_json (ChatbotMessageArray *message_array,
                               JsonBuilder *builder)
{
  g_return_if_fail (message_array != NULL);
  g_return_if_fail (JSON_IS_BUILDER (builder));

  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "length");
  json_builder_add_int_value (builder, message_array->fifo->length);
  json_builder_set_member_name (builder, "messages");
  json_builder_begin_array (builder);
  g_queue_foreach (message_array->fifo, _message_to_json, builder);
  json_builder_end_array (builder);
  json_builder_end_object (builder);
}

/**
 * chatbot_message_array_at:
 * @message_array: self
 * @index: index
 *
 * Gets message at given @index.
 *
 * Returns: (transfer none): message at @index
 */
ChatbotMessage *
chatbot_message_array_at (ChatbotMessageArray *message_array, guint index)
{
  g_return_val_if_fail (message_array != NULL, NULL);
  return g_queue_peek_nth (message_array->fifo, index);
}

/**
 * chatbot_message_array_length:
 * @message_array: self
 *
 * Gets length of message array.
 *
 * Returns: Number of items @self holds.
 */
gsize
chatbot_message_array_length (ChatbotMessageArray *message_array)
{
  g_return_val_if_fail (message_array != NULL, 0);
  return g_queue_get_length (message_array->fifo);
}

/**
 * chatbot_message_array_push:
 * @message_array: self
 * @message: message
 *
 * Pushes @message to @message_array.
 */
void
chatbot_message_array_push (ChatbotMessageArray *message_array,
                            ChatbotMessage *message)
{
  g_return_if_fail (message_array != NULL);
  g_return_if_fail (message != NULL);

  g_object_ref (message);
  g_queue_push_tail (message_array->fifo, message);
}

/**
 * chatbot_message_array_push_take:
 * @message_array: self
 * @message: (transfer full): message
 *
 * Same as [method@Chatbot.MessageArray.push] but takes ownership of @message.
 */
void
chatbot_message_array_push_take (ChatbotMessageArray *message_array,
                                 ChatbotMessage *message)
{
  g_return_if_fail (message_array != NULL);
  g_return_if_fail (message != NULL);

  g_queue_push_tail (message_array->fifo, message);
}

/**
 * chatbot_message_array_pop:
 * @message_array: self
 *
 * Pops message from @message_array.
 *
 * Returns: (nullable) (transfer full): Popped element or %NULL if no element
 * left.
 */
ChatbotMessage *
chatbot_message_array_pop (ChatbotMessageArray *message_array)
{
  g_return_val_if_fail (message_array != NULL, NULL);
  return g_queue_pop_head (message_array->fifo);
}
