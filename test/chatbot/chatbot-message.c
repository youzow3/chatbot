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

#include <chatbot/chatbot.h>
#include <test/chatbot/chatbot-test.h>

static void
test_ctor (gconstpointer user_data)
{
  ChatbotData *data;
  ChatbotMessage *msg;

  g_return_if_fail (CHATBOT_IS_DATA ((gpointer)user_data));

  data = CHATBOT_DATA ((gpointer)user_data);
  msg = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, data);
  g_assert_nonnull (msg);
  g_assert_true (chatbot_message_is_valid (msg, NULL));
  g_object_unref (msg);
}

static void
test_ctor_from_variant (gconstpointer user_data)
{
  GVariant *variant;
  ChatbotMessage *msg;

  g_return_if_fail (user_data != NULL);
  variant = (GVariant *)user_data;
  msg = chatbot_message_new_from_variant (variant);
  g_assert_nonnull (msg);
  g_assert_true (chatbot_message_is_valid (msg, NULL));
  g_object_unref (msg);
}

static void
test_ctor_from_variant_fail (gconstpointer user_data)
{
  GVariant *variant;
  ChatbotMessage *msg;
  GError *error = NULL;

  g_return_if_fail (user_data != NULL);
  variant = (GVariant *)user_data;
  msg = chatbot_message_new_from_variant (variant);
  g_assert_nonnull (msg);
  g_assert_true (!chatbot_message_is_valid (msg, &error));
  g_assert_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID);
  g_error_free (error);
  g_object_unref (msg);
}

static void
test_ctor_from_json (gconstpointer user_data)
{
  JsonReader *reader;
  ChatbotMessage *msg;

  g_return_if_fail (JSON_IS_READER (user_data));
  reader = JSON_READER (user_data);

  msg = chatbot_message_new_from_json (reader);
  g_assert_nonnull (msg);
  g_assert_true (chatbot_message_is_valid (msg, NULL));
  g_object_unref (msg);
}

static void
test_ctor_from_json_fail (gconstpointer user_data)
{
  JsonReader *reader;
  ChatbotMessage *msg;
  GError *error = NULL;

  g_return_if_fail (JSON_IS_READER (user_data));
  reader = JSON_READER (user_data);

  msg = chatbot_message_new_from_json (reader);
  g_assert_nonnull (msg);
  g_assert_true (!chatbot_message_is_valid (msg, &error));
  g_assert_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID);
  g_error_free (error);
  g_object_unref (msg);
}

static void
test_get_role (gconstpointer user_data)
{
  ChatbotMessage *msg;
  ChatbotData *data;

  g_return_if_fail (CHATBOT_IS_DATA ((gpointer)user_data));
  data = CHATBOT_DATA ((gpointer)user_data);

  msg = chatbot_message_new (CHATBOT_MESSAGE_ROLE_USER, data);
  g_assert_true (chatbot_message_get_role (msg) == CHATBOT_MESSAGE_ROLE_USER);
  g_object_unref (msg);
}

static void
test_get_data (gconstpointer user_data)
{
  ChatbotMessage *msg;
  ChatbotData *data;

  g_return_if_fail (CHATBOT_IS_DATA ((gpointer)user_data));
  data = CHATBOT_DATA ((gpointer)user_data);

  msg = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, data);
  g_assert_true (chatbot_message_get_data (msg) == data);
}

static void
test_to_variant (gconstpointer user_data)
{
  ChatbotMessage *msg;
  ChatbotData *data;
  GVariant *variant;
  GVariantDict vdict;
  gint32 role;
  const gchar *mime_type;
  const gchar *msg_data;

  g_return_if_fail (CHATBOT_IS_DATA ((gpointer)user_data));
  data = CHATBOT_DATA ((gpointer)user_data);
  msg = chatbot_message_new (CHATBOT_MESSAGE_ROLE_USER, data);
  variant = chatbot_message_to_variant (msg);
  g_variant_dict_init (&vdict, variant);
  g_assert_true (g_variant_dict_lookup (&vdict, "role", "i", &role));
  g_assert_true (
      g_variant_dict_lookup (&vdict, "mime-type", "&s", &mime_type));
  g_assert_true (g_variant_dict_lookup (&vdict, "data", "^&ay", &msg_data));

  g_assert_true (role == CHATBOT_MESSAGE_ROLE_USER);
  g_assert_true (g_str_equal (mime_type, CHATBOT_TEST_DATA_MIME_TYPE));
  g_assert_true (g_str_equal (msg_data, CHATBOT_TEST_DATA_DATA));

  g_variant_dict_end (&vdict);
  g_object_unref (msg);
}

static void
test_to_json (gconstpointer user_data)
{
  ChatbotMessage *msg;
  ChatbotData *data;
  gint32 role;
  const gchar *mime_type;
  const gchar *msg_data;
  JsonBuilder *builder;
  JsonReader *reader;

  g_return_if_fail (CHATBOT_IS_DATA ((gpointer)user_data));
  data = CHATBOT_DATA ((gpointer)user_data);
  msg = chatbot_message_new (CHATBOT_MESSAGE_ROLE_USER, data);
  builder = json_builder_new ();
  chatbot_message_to_json (msg, builder);
  reader = json_reader_new (json_builder_get_root (builder));
  g_assert_true (json_reader_read_member (reader, "role"));
  role = (gint32)json_reader_get_int_value (reader);
  json_reader_end_member (reader);
  g_assert_true (json_reader_read_member (reader, "mime-type"));
  mime_type = json_reader_get_string_value (reader);
  json_reader_end_member (reader);
  g_assert_true (json_reader_read_member (reader, "data"));
  msg_data = json_reader_get_string_value (reader);
  json_reader_end_member (reader);

  g_assert_true (role == CHATBOT_MESSAGE_ROLE_USER);
  g_assert_true (g_str_equal (mime_type, CHATBOT_TEST_DATA_MIME_TYPE));
  g_assert_true (g_str_equal (msg_data, CHATBOT_TEST_DATA_DATA));

  g_object_unref (reader);
  g_object_unref (builder);
  g_object_unref (msg);
}

static JsonReader *
_json_parse (const gchar *json)
{
  JsonParser *parser;
  JsonReader *reader;

  parser = json_parser_new ();
  g_assert_true (json_parser_load_from_data (parser, json, -1, NULL));
  reader = json_reader_new (json_parser_get_root (parser));
  return reader;
}

int
main (int argc, char **argv)
{
  ChatbotData *data;
  GVariant *data_variant;
  GVariant *data_variant_role_missing;
  GVariant *data_variant_mime_type_missing;
  GVariant *data_variant_data_missing;
  JsonReader *reader;
  JsonReader *reader_role_missing;
  JsonReader *reader_mime_type_missing;
  JsonReader *reader_data_missing;
  int ret;

  g_test_init (&argc, &argv, NULL);
  chatbot_data_iface_register_type (CHATBOT_TEST_DATA_MIME_TYPE,
                                    CHATBOT_TYPE_TEST_DATA);
  data = chatbot_data_new_from_data (CHATBOT_TEST_DATA_MIME_TYPE,
                                     CHATBOT_TEST_DATA_DATA,
                                     sizeof (CHATBOT_TEST_DATA_DATA), NULL);
  data_variant = g_variant_parse (
      G_VARIANT_TYPE_VARDICT,
      "{\"role\": <1>, \"mime-type\": <\"" CHATBOT_TEST_DATA_MIME_TYPE
      "\">, \"data\": <b\"" CHATBOT_TEST_DATA_DATA "\">}",
      NULL, NULL, NULL);
  g_assert (data_variant != NULL);
  data_variant_role_missing
      = g_variant_parse (G_VARIANT_TYPE_VARDICT,
                         "{\"mime-type\": <\"" CHATBOT_TEST_DATA_MIME_TYPE
                         "\">, \"data\": <b\"" CHATBOT_TEST_DATA_DATA "\">}",
                         NULL, NULL, NULL);
  g_assert (data_variant_role_missing != NULL);
  data_variant_mime_type_missing = g_variant_parse (
      G_VARIANT_TYPE_VARDICT,
      "{\"role\": <1>, \"data\": <b\"" CHATBOT_TEST_DATA_DATA "\">}", NULL,
      NULL, NULL);
  g_assert (data_variant_mime_type_missing != NULL);
  data_variant_data_missing = g_variant_parse (
      G_VARIANT_TYPE_VARDICT,
      "{\"role\": <1>, \"mime-type\": <\"" CHATBOT_TEST_DATA_MIME_TYPE "\">}",
      NULL, NULL, NULL);
  g_assert (data_variant_data_missing != NULL);
  reader = _json_parse (
      "{\"role\": 0, \"mime-type\": \"" CHATBOT_TEST_DATA_MIME_TYPE
      "\", \"data\": \"" CHATBOT_TEST_DATA_DATA "\"}");
  reader_role_missing
      = _json_parse ("{\"mime-type\": \"" CHATBOT_TEST_DATA_MIME_TYPE
                     "\", \"data\": \"" CHATBOT_TEST_DATA_DATA "\"}");
  reader_mime_type_missing = _json_parse (
      "{\"role\": 0, \"data\": \"" CHATBOT_TEST_DATA_DATA "\"}");
  reader_data_missing = _json_parse (
      "{\"role\": 0, \"mime-type\": \"" CHATBOT_TEST_DATA_MIME_TYPE "\"}");

  g_test_add_data_func ("/chatbot-message/ctor", data, test_ctor);
  g_test_add_data_func ("/chatbot-message/ctor/from_variant", data_variant,
                        test_ctor_from_variant);
  g_test_add_data_func ("/chatbot-message/ctor/from_variant/fail/role-missing",
                        data_variant_role_missing,
                        test_ctor_from_variant_fail);
  g_test_add_data_func (
      "/chatbot-message/ctor/from_variant/fail/mime_type-missing",
      data_variant_mime_type_missing, test_ctor_from_variant_fail);
  g_test_add_data_func ("/chatbot-message/ctor/from_variant/fail/data-missing",
                        data_variant_data_missing,
                        test_ctor_from_variant_fail);
  g_test_add_data_func ("/chatbot-message/ctor/from_json", reader,
                        test_ctor_from_json);
  g_test_add_data_func ("/chatbot-message/ctor/from_json/fail/role-missing",
                        reader_role_missing, test_ctor_from_json_fail);
  g_test_add_data_func (
      "/chatbot-message/ctor/from_json/fail/mime_type-missing",
      reader_mime_type_missing, test_ctor_from_json_fail);
  g_test_add_data_func ("/chatbot-message/ctor/from_json/fail/data-missing",
                        reader_data_missing, test_ctor_from_json_fail);
  g_test_add_data_func ("/chatbot-message/get_role", data, test_get_role);
  g_test_add_data_func ("/chatbot-message/get_data", data, test_get_data);
  g_test_add_data_func ("/chatbot-message/to_variant", data, test_to_variant);
  g_test_add_data_func ("/chatbot-message/to_json", data, test_to_json);
  ret = g_test_run ();
  g_object_unref (reader_data_missing);
  g_object_unref (reader_mime_type_missing);
  g_object_unref (reader_role_missing);
  g_object_unref (reader);
  g_variant_unref (data_variant_data_missing);
  g_variant_unref (data_variant_mime_type_missing);
  g_variant_unref (data_variant_role_missing);
  g_variant_unref (data_variant);
  g_object_unref (data);
  return ret;
}
