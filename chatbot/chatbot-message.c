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
 * ChatbotMessage:
 *
 * Opaque structure representing chat message.
 */

#include <chatbot/chatbot-error.h>

#include <chatbot/chatbot-message.h>

enum
{
  PROP_ROLE = 1,
  PROP_MIME_TYPE,
  PROP_DATA,
  PROP_VARIANT,
  PROP_JSON,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = {
  NULL,
};

struct _ChatbotMessage
{
  GObject parent;

  ChatbotMessageRole role;
  gchar *mime_type;
  ChatbotData *data;
  GVariant *variant;
  JsonReader *json;
  GError *parse_error;
};

G_DEFINE_FINAL_TYPE (ChatbotMessage, chatbot_message, G_TYPE_OBJECT);

static void
chatbot_message_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  ChatbotMessage *message;

  g_return_if_fail (CHATBOT_IS_MESSAGE (object));
  message = CHATBOT_MESSAGE (object);

  switch (property_id)
    {
    case PROP_ROLE:
      message->role = g_value_get_enum (value);
      break;
    case PROP_MIME_TYPE:
      g_clear_pointer (&message->mime_type, g_free);
      message->mime_type = g_value_dup_string (value);
      break;
    case PROP_DATA:
      message->data = g_value_dup_object (value);
      break;
    case PROP_VARIANT:
      g_clear_pointer (&message->variant, g_variant_unref);
      message->variant = g_value_dup_variant (value);
      break;
    case PROP_JSON:
      g_clear_object (&message->json);
      message->json = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
chatbot_message_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  ChatbotMessage *message;

  g_return_if_fail (CHATBOT_IS_MESSAGE (object));
  message = CHATBOT_MESSAGE (object);

  switch (property_id)
    {
    case PROP_ROLE:
      g_value_set_enum (value, message->role);
      break;
    case PROP_MIME_TYPE:
      g_value_set_string (value, message->mime_type);
      break;
    case PROP_DATA:
      g_value_set_object (value, message->data);
      break;
    case PROP_VARIANT:
      g_value_set_variant (value, chatbot_message_to_variant (message));
      break;
    }
  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
chatbot_message_dispose (GObject *object)
{
  ChatbotMessage *message;

  g_return_if_fail (CHATBOT_IS_MESSAGE (object));
  message = CHATBOT_MESSAGE (object);

  g_clear_pointer (&message->variant, g_variant_unref);
  g_clear_object (&message->data);

  G_OBJECT_CLASS (chatbot_message_parent_class)->dispose (object);
}

static void
chatbot_message_finalize (GObject *object)
{
  ChatbotMessage *message;

  g_return_if_fail (CHATBOT_IS_MESSAGE (object));
  message = CHATBOT_MESSAGE (object);

  g_free (message->mime_type);
  if (message->parse_error)
    g_error_free (message->parse_error);

  G_OBJECT_CLASS (chatbot_message_parent_class)->finalize (object);
}

static gboolean
_from_variant (ChatbotMessage *message, GVariant *variant, GError **error)
{
  GVariantDict *dict = NULL;
  GVariant *data = NULL;
  const guint8 *data_ptr;
  gsize data_size;
  ChatbotData *data_object;

  gboolean ret = FALSE;

  g_return_val_if_fail (CHATBOT_IS_MESSAGE (message), FALSE);
  g_return_val_if_fail (variant != NULL, FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  dict = g_variant_dict_new (message->variant);
  if (!g_variant_dict_lookup (dict, "role", "i", &message->role))
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Given variant doesn't contain \"role\".");
      goto cleanup;
    }

  if (!g_variant_dict_lookup (dict, "mime-type", "s", &message->mime_type))
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Given variant doesn't contain \"mime-type\".");
      goto cleanup;
    }

  if (!g_variant_dict_lookup (dict, "data", "@ay", &data))
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Given variant doesn't contain \"data\".");
      goto cleanup;
    }

  data_ptr = g_variant_get_fixed_array (data, &data_size, sizeof (guint8));
  data_object = chatbot_data_new_from_data (message->mime_type, data_ptr,
                                            data_size, error);
  if (data_object == NULL)
    goto cleanup;
  message->data = data_object;
  ret = TRUE;
cleanup:
  if (data != NULL)
    g_variant_unref (data);
  if (dict != NULL)
    g_variant_dict_unref (dict);
  return ret;
}

static gboolean
_from_json (ChatbotMessage *message, JsonReader *reader, GError **error)
{
  const gchar *data;
  gboolean ret = FALSE;

  g_return_val_if_fail (CHATBOT_IS_MESSAGE (message), FALSE);
  g_return_val_if_fail (
      JSON_IS_READER (reader) && json_reader_is_object (reader), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  if (!json_reader_read_member (message->json, "role"))
    {
      json_reader_end_member (message->json);
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Given JSON doesn't contain \"role\".");
      goto cleanup;
    }
  message->role = json_reader_get_int_value (message->json);
  json_reader_end_member (message->json); // read_member("role")

  if (!json_reader_read_member (message->json, "mime-type"))
    {
      json_reader_end_member (message->json);
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Given JSON doesn't contain \"mime-type\".");
      goto cleanup;
    }
  message->mime_type = g_strdup (json_reader_get_string_value (message->json));
  json_reader_end_member (message->json); // read_member("mime-type")

  if (!json_reader_read_member (message->json, "data"))
    {
      json_reader_end_member (message->json);
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Given JSON doesn't contain \"data\".");
      goto cleanup;
    }
  data = json_reader_get_string_value (message->json);
  message->data = chatbot_data_new_from_text (message->mime_type, data, error);
  json_reader_end_member (message->json); // "read_member("data")
  if (message->data == NULL)
    goto cleanup;

  ret = TRUE;
cleanup:
  return ret;
}

static void
chatbot_message_constructed (GObject *object)
{
  ChatbotMessage *message;
  GError *error = NULL;

  g_return_if_fail (CHATBOT_IS_MESSAGE (object));
  message = CHATBOT_MESSAGE (object);

  if (message->data != NULL)
    {
      const gchar *mime_type = chatbot_data_iface_mime_type_from_gtype (
          G_OBJECT_TYPE (message->data));
      if (mime_type == NULL)
        {
          g_critical ("Not registered data class is used.");
          return;
        }
      message->mime_type = g_strdup (mime_type);
      return;
    }
  g_warn_if_fail (message->mime_type == NULL);
  g_warn_if_fail (message->data == NULL);

  if (message->variant)
    {
      g_warn_if_fail (!JSON_IS_READER (message->json));
      if (!_from_variant (message, message->variant, &error))
        g_propagate_error (&message->parse_error, error);
      goto cleanup;
    }

  if (message->json)
    {
      if (!_from_json (message, message->json, &error))
        g_propagate_error (&message->parse_error, error);
      goto cleanup;
    }
cleanup:
  g_clear_pointer (&message->variant, g_variant_unref);
  g_clear_object (&message->json);

  if (message->parse_error != NULL)
    return;
  if ((message->mime_type == NULL) || (message->data == NULL))
    message->parse_error = g_error_new (CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                                        "The message is incomplete.");
}

static void
chatbot_message_class_init (ChatbotMessageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = chatbot_message_set_property;
  object_class->get_property = chatbot_message_get_property;
  object_class->dispose = chatbot_message_dispose;
  object_class->finalize = chatbot_message_finalize;
  object_class->constructed = chatbot_message_constructed;

  /**
   * ChatbotMessage:role:
   *
   * Chat role.
   *
   * [enum@Chatbot.MessageRole.NONE] for non-chat use.
   */
  properties[PROP_ROLE] = g_param_spec_enum (
      "role", "role", "chat role", CHATBOT_TYPE_MESSAGE_ROLE,
      CHATBOT_MESSAGE_ROLE_NONE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * ChatbotMessage:mime-type:
   *
   * Message MIME type.
   */
  properties[PROP_MIME_TYPE]
      = g_param_spec_string ("mime-type", "mime-type", "MIME type", NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * ChatbotMessage:data:
   *
   * Message data in [iface@Chatbot.Data] form.
   */
  properties[PROP_DATA]
      = g_param_spec_object ("data", "data", "data object", CHATBOT_TYPE_DATA,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * ChatbotMessage:variant:
   *
   * #GVariant representing message.
   */
  properties[PROP_VARIANT] = g_param_spec_variant (
      "variant", "variant", "variant", G_VARIANT_TYPE_VARDICT, NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * ChatbotMessage:json:
   *
   * [class@Json.Reader] to construct [class@Chatbot.Message].
   *
   * This property is used only for constructing object.
   */
  properties[PROP_JSON]
      = g_param_spec_object ("json", "json", "json", JSON_TYPE_READER,
                             G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
chatbot_message_init (ChatbotMessage *message)
{
}

/**
 * chatbot_message_new:
 * @role: role
 * @data: object representing data
 *
 * Constructs [class@Chatbot.Message] from given @role and @data.
 *
 * The class of @data should be registered with
 * [func@Chatbot.Data.iface_register_type].
 *
 * Returns: Newly created instance.
 */
ChatbotMessage *
chatbot_message_new (ChatbotMessageRole role, ChatbotData *data)
{
  g_return_val_if_fail (CHATBOT_IS_DATA (data), NULL);
  return g_object_new (CHATBOT_TYPE_MESSAGE, "role", role, "data", data, NULL);
}

/**
 * chatbot_message_new_from_variant:
 * @variant: #GVariant with type `a{sv}`
 *
 * Constructs [class@Chatbot.Message] from @variant.
 *
 * The variant must be dictionary, and have elements `role`, `mime-type` and
 * `data`.
 *
 * The returned value maybe invalid message. Use
 * [method@Chatbot.Message.is_valid] to check whether the message is valid.
 *
 * Returns: Newly created message from @variant.
 */
ChatbotMessage *
chatbot_message_new_from_variant (GVariant *variant)
{
  g_return_val_if_fail (variant != NULL, NULL);
  return g_object_new (CHATBOT_TYPE_MESSAGE, "variant", variant, NULL);
}

/**
 * chatbot_message_new_from_json:
 * @reader: JSON reader
 *
 * Constructs [class@Chatbot.Message] from @reader.
 *
 * The JSON must have elements `role`, `mime-type` and `data`.
 *
 * The returned value maybe invalid message. Use
 * [method@Chatbot.Message.is_valid] to check whether the message is valid.
 *
 * Returns: Newly created message from @reader.
 */
ChatbotMessage *
chatbot_message_new_from_json (JsonReader *reader)
{
  g_return_val_if_fail (JSON_IS_READER (reader), NULL);
  return g_object_new (CHATBOT_TYPE_MESSAGE, "json", reader, NULL);
}

/**
 * chatbot_message_is_valid:
 * @message: self
 * @error: (nullable): location to store error
 *
 * Checks whether @message is valid.
 *
 * If @message is constructed with [ctor@Chatbot.Message.new], this must always
 * return %TRUE.
 *
 * Returns: %TRUE if @message is valid, and %FALSE with @error if @message is
 * invalid.
 */
gboolean
chatbot_message_is_valid (ChatbotMessage *message, GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_MESSAGE (message), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  if (message->parse_error == NULL)
    return TRUE;
  if (error)
    *error = g_error_copy (message->parse_error);
  return FALSE;
}

/**
 * chatbot_message_get_role:
 * @message: self
 *
 * Gets role.
 *
 * Returns: Role of the message.
 */
ChatbotMessageRole
chatbot_message_get_role (ChatbotMessage *message)
{
  g_return_val_if_fail (CHATBOT_IS_MESSAGE (message),
                        CHATBOT_MESSAGE_ROLE_NONE);
  return message->role;
}

/**
 * chatbot_message_get_mime_type:
 * @message: self
 *
 * Gets MIME type.
 *
 * Returned value could be %NULL only if @message is incomplete and doesn't
 * have MIME type.
 *
 * Returns: (nullable): MIME type of the message.
 */
const gchar *
chatbot_message_get_mime_type (ChatbotMessage *message)
{
  g_return_val_if_fail (CHATBOT_IS_MESSAGE (message), NULL);
  return message->mime_type;
}

/**
 * chatbot_message_get_data:
 * @message: self
 *
 * Gets data.
 *
 * Returned value could be %NULL if @message is incomplete.
 *
 * Returns: (nullable) (transfer none): Data of the message.
 */
ChatbotData *
chatbot_message_get_data (ChatbotMessage *message)
{
  g_return_val_if_fail (CHATBOT_IS_MESSAGE (message), NULL);
  return message->data;
}

/**
 * chatbot_message_to_variant:
 * @message: self
 *
 * Converts to approriate #GVariant representation.
 *
 * Returns: (transfer none): #GVariant representing @message.
 */
GVariant *
chatbot_message_to_variant (ChatbotMessage *message)
{
  GVariantDict vd;
  gsize data_size;
  gconstpointer data;

  g_return_val_if_fail (CHATBOT_IS_MESSAGE (message)
                            && chatbot_message_is_valid (message, NULL),
                        NULL);

  if (message->variant != NULL)
    return message->variant;

  g_variant_dict_init (&vd, NULL);
  g_variant_dict_insert (&vd, "role", "i", message->role);
  g_variant_dict_insert (&vd, "mime-type", "s", message->mime_type);
  data = chatbot_data_get_data (message->data, &data_size);
  g_variant_dict_insert_value (&vd, "data",
                               g_variant_new_fixed_array (G_VARIANT_TYPE_BYTE,
                                                          data, data_size,
                                                          sizeof (guint8)));
  g_object_ref (message->data);
  message->variant = g_variant_dict_end (&vd);
  g_variant_ref_sink (message->variant);
  return message->variant;
}

/**
 * chatbot_message_to_json:
 * @builder: Json builder
 *
 * Converts to JSON representation.
 */
void
chatbot_message_to_json (ChatbotMessage *message, JsonBuilder *builder)
{
  g_return_if_fail (CHATBOT_IS_MESSAGE (message)
                    && chatbot_message_is_valid (message, NULL));
  g_return_if_fail (JSON_IS_BUILDER (builder));

  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "role");
  json_builder_add_int_value (builder, message->role);
  json_builder_set_member_name (builder, "mime-type");
  json_builder_add_string_value (builder, message->mime_type);
  json_builder_set_member_name (builder, "data");
  json_builder_add_string_value (builder,
                                 chatbot_data_get_text (message->data));
  json_builder_end_object (builder);
}
