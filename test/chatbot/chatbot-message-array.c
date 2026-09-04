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
test_ctor (void)
{
  ChatbotMessageArray *msg = chatbot_message_array_new ();
  g_assert_nonnull (msg);
  chatbot_message_array_free (msg);
}

static void
test_ctor_from_variant (gconstpointer user_data)
{
  g_return_if_fail (user_data != NULL);

  GVariant *variant = (GVariant *)user_data;
  ChatbotMessageArray *message_array
      = chatbot_message_array_new_from_variant (variant, NULL);
  ChatbotMessage *msg1;
  ChatbotData *data1;
  ChatbotMessage *msg2;
  ChatbotData *data2;
  ChatbotMessage *msg3;
  ChatbotData *data3;

  g_assert_nonnull (message_array);
  g_assert_true (chatbot_message_array_length (message_array) == 3);
  msg1 = chatbot_message_array_pop (message_array);
  g_assert_nonnull (msg1);
  g_assert_true (chatbot_message_get_role (msg1)
                 == CHATBOT_MESSAGE_ROLE_SYSTEM);
  data1 = chatbot_message_get_data (msg1);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (data1));
  g_assert_true (g_str_equal (chatbot_data_get_text (data1), "SYSTEM"));

  msg2 = chatbot_message_array_pop (message_array);
  g_assert_nonnull (msg2);
  g_assert_true (chatbot_message_get_role (msg2) == CHATBOT_MESSAGE_ROLE_USER);
  data2 = chatbot_message_get_data (msg2);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (data2));
  g_assert_true (g_str_equal (chatbot_data_get_text (data2), "USER"));

  msg3 = chatbot_message_array_pop (message_array);
  g_assert_nonnull (msg3);
  g_assert_true (chatbot_message_get_role (msg3)
                 == CHATBOT_MESSAGE_ROLE_ASSISTANT);
  data3 = chatbot_message_get_data (msg3);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (data3));
  g_assert_true (g_str_equal (chatbot_data_get_text (data3), "ASSISTANT"));

  g_object_unref (msg3);
  g_object_unref (msg2);
  g_object_unref (msg1);
  chatbot_message_array_free (message_array);
}

static void
test_ctor_from_json (gconstpointer user_data)
{
  g_return_if_fail (JSON_IS_READER ((gpointer)user_data));

  JsonReader *reader = JSON_READER ((gpointer)user_data);
  ChatbotMessageArray *message_array
      = chatbot_message_array_new_from_json (reader, NULL);
  ChatbotMessage *msg1;
  ChatbotData *data1;
  ChatbotMessage *msg2;
  ChatbotData *data2;
  ChatbotMessage *msg3;
  ChatbotData *data3;

  g_assert_nonnull (message_array);
  g_assert_cmpint (chatbot_message_array_length (message_array), ==, 3);
  msg1 = chatbot_message_array_pop (message_array);
  g_assert_nonnull (msg1);
  g_assert_true (chatbot_message_get_role (msg1)
                 == CHATBOT_MESSAGE_ROLE_SYSTEM);
  data1 = chatbot_message_get_data (msg1);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (data1));
  g_assert_true (g_str_equal (chatbot_data_get_text (data1), "SYSTEM"));

  msg2 = chatbot_message_array_pop (message_array);
  g_assert_nonnull (msg2);
  g_assert_true (chatbot_message_get_role (msg2) == CHATBOT_MESSAGE_ROLE_USER);
  data2 = chatbot_message_get_data (msg2);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (data2));
  g_assert_true (g_str_equal (chatbot_data_get_text (data2), "USER"));

  msg3 = chatbot_message_array_pop (message_array);
  g_assert_nonnull (msg3);
  g_assert_true (chatbot_message_get_role (msg3)
                 == CHATBOT_MESSAGE_ROLE_ASSISTANT);
  data3 = chatbot_message_get_data (msg3);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (data3));
  g_assert_true (g_str_equal (chatbot_data_get_text (data3), "ASSISTANT"));

  g_object_unref (msg3);
  g_object_unref (msg2);
  g_object_unref (msg1);
  chatbot_message_array_free (message_array);
}

static void
test_copy (void)
{
  ChatbotMessageArray *message_array = chatbot_message_array_new ();
  ChatbotTestData *data = chatbot_test_data_new ();
  ChatbotMessage *msg1
      = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, CHATBOT_DATA (data));
  ChatbotMessage *msg2
      = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, CHATBOT_DATA (data));
  ChatbotMessage *msg3
      = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, CHATBOT_DATA (data));
  ChatbotMessage *msg4
      = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, CHATBOT_DATA (data));
  ChatbotMessageArray *copied;
  ChatbotMessage *copied_popped1;

  chatbot_message_array_push (message_array, msg1);
  chatbot_message_array_push (message_array, msg2);
  chatbot_message_array_push (message_array, msg3);
  chatbot_message_array_push (message_array, msg4);

  copied = chatbot_message_array_copy (message_array);
  g_assert_nonnull (copied);

  chatbot_message_array_length (copied);
  g_assert_cmpint (chatbot_message_array_length (copied), ==, 4);
  copied_popped1 = chatbot_message_array_pop (copied);
  g_assert_true (copied_popped1 == msg1);
  g_assert_cmpint (chatbot_message_array_length (message_array), ==, 4);
  g_assert_cmpint (chatbot_message_array_length (copied), ==, 3);

  g_object_unref (copied_popped1);
  chatbot_message_array_free (copied);
  g_object_unref (msg4);
  g_object_unref (msg3);
  g_object_unref (msg2);
  g_object_unref (msg1);
  g_object_unref (data);
  chatbot_message_array_free (message_array);
}

static void
test_to_variant (void)
{
  ChatbotMessageArray *message_array;
  ChatbotTextPlain *data1;
  ChatbotTextPlain *data2;
  ChatbotMessage *msg1;
  ChatbotMessage *msg2;
  GVariant *variant;
  GVariantIter *iter;
  GVariant *dict;
  ChatbotMessage *rec_msg1;
  ChatbotMessage *rec_msg2;

  message_array = chatbot_message_array_new ();
  data1 = chatbot_text_plain_new ("one");
  data2 = chatbot_text_plain_new ("two");
  msg1 = chatbot_message_new (CHATBOT_MESSAGE_ROLE_USER, CHATBOT_DATA (data1));
  msg2 = chatbot_message_new (CHATBOT_MESSAGE_ROLE_ASSISTANT,
                              CHATBOT_DATA (data2));
  chatbot_message_array_push (message_array, msg1);
  chatbot_message_array_push (message_array, msg2);

  variant = chatbot_message_array_to_variant (message_array);
  g_variant_ref_sink (variant);
  g_assert_true (g_variant_type_equal (g_variant_get_type (variant),
                                       G_VARIANT_TYPE ("aa{sv}")));
  g_assert_cmpint (g_variant_n_children (variant), ==, 2);
  iter = g_variant_iter_new (variant);

  g_assert_true (g_variant_iter_next (iter, "@a{sv}", &dict));
  rec_msg1 = chatbot_message_new_from_variant (dict);
  g_assert_true (CHATBOT_IS_MESSAGE (rec_msg1));
  g_assert_true (chatbot_message_get_role (rec_msg1)
                 == CHATBOT_MESSAGE_ROLE_USER);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (chatbot_message_get_data (rec_msg1)));
  g_assert_cmpstr (chatbot_text_plain_get_text (CHATBOT_TEXT_PLAIN (
                       chatbot_message_get_data (rec_msg1))),
                   ==, "one");
  g_variant_unref (dict);

  g_assert_true (g_variant_iter_next (iter, "@a{sv}", &dict));
  rec_msg2 = chatbot_message_new_from_variant (dict);
  g_assert_true (CHATBOT_IS_MESSAGE (rec_msg2));
  g_assert_true (chatbot_message_get_role (rec_msg2)
                 == CHATBOT_MESSAGE_ROLE_ASSISTANT);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (chatbot_message_get_data (rec_msg2)));
  g_assert_cmpstr (chatbot_text_plain_get_text (CHATBOT_TEXT_PLAIN (
                       chatbot_message_get_data (rec_msg2))),
                   ==, "two");
  g_variant_unref (dict);

  g_object_unref (rec_msg2);
  g_object_unref (rec_msg1);
  g_variant_iter_free (iter);
  g_variant_unref (variant);
  g_object_unref (msg2);
  g_object_unref (msg1);
  g_object_unref (data2);
  g_object_unref (data1);
  chatbot_message_array_free (message_array);
}

static void
test_to_json (void)
{
  ChatbotMessageArray *message_array;
  ChatbotTextPlain *data1;
  ChatbotTextPlain *data2;
  ChatbotMessage *msg1;
  ChatbotMessage *msg2;
  ChatbotMessage *rec_msg1;
  ChatbotMessage *rec_msg2;
  JsonBuilder *builder;
  JsonReader *reader;
  JsonReader *message_reader;

  message_array = chatbot_message_array_new ();
  data1 = chatbot_text_plain_new ("one");
  data2 = chatbot_text_plain_new ("two");
  msg1 = chatbot_message_new (CHATBOT_MESSAGE_ROLE_USER, CHATBOT_DATA (data1));
  msg2 = chatbot_message_new (CHATBOT_MESSAGE_ROLE_ASSISTANT,
                              CHATBOT_DATA (data2));
  chatbot_message_array_push (message_array, msg1);
  chatbot_message_array_push (message_array, msg2);

  builder = json_builder_new ();
  chatbot_message_array_to_json (message_array, builder);

  reader = json_reader_new (json_builder_get_root (builder));
  g_assert_true (json_reader_read_member (reader, "length"));
  g_assert_cmpint (json_reader_get_int_value (reader), ==, 2);
  json_reader_end_member (reader);

  g_assert_true (json_reader_read_member (reader, "messages"));
  g_assert_true(json_reader_is_array(reader));

  g_assert_cmpint (json_reader_count_elements (reader), ==, 2);

  g_assert_true (json_reader_read_element (reader, 0));
  message_reader = json_reader_new(json_reader_get_current_node(reader));
  json_reader_end_element(reader);
  rec_msg1 = chatbot_message_new_from_json (message_reader);
  g_assert_nonnull (rec_msg1);
  g_assert_true (chatbot_message_get_role (rec_msg1)
                 == CHATBOT_MESSAGE_ROLE_USER);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (chatbot_message_get_data (rec_msg1)));
  g_assert_cmpstr (chatbot_text_plain_get_text (CHATBOT_TEXT_PLAIN (
                       chatbot_message_get_data (rec_msg1))),
                   ==, "one");
  g_object_unref(message_reader);

  g_assert_true (json_reader_read_element (reader, 1));
  message_reader = json_reader_new(json_reader_get_current_node(reader));
  json_reader_end_element(reader);
  rec_msg2 = chatbot_message_new_from_json (message_reader);
  g_assert_nonnull (rec_msg2);
  g_assert_true (chatbot_message_get_role (rec_msg2)
                 == CHATBOT_MESSAGE_ROLE_ASSISTANT);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (chatbot_message_get_data (rec_msg2)));
  g_assert_cmpstr (chatbot_text_plain_get_text (CHATBOT_TEXT_PLAIN (
                       chatbot_message_get_data (rec_msg2))),
                   ==, "two");
  g_object_unref (message_reader);

  g_object_unref (rec_msg2);
  g_object_unref (rec_msg1);
  g_object_unref (reader);
  g_object_unref (msg2);
  g_object_unref (msg1);
  chatbot_message_array_free (message_array);
}

static void
test_push__at__length__pop (void)
{
  ChatbotMessageArray *message_array = chatbot_message_array_new ();
  ChatbotTestData *data = chatbot_test_data_new ();
  ChatbotMessage *message1
      = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, CHATBOT_DATA (data));
  ChatbotMessage *message2
      = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, CHATBOT_DATA (data));
  ChatbotMessage *message3
      = chatbot_message_new (CHATBOT_MESSAGE_ROLE_NONE, CHATBOT_DATA (data));
  ChatbotMessage *message1_popped;
  ChatbotMessage *message2_popped;
  ChatbotMessage *message3_popped;

  g_assert_true (chatbot_message_array_length (message_array) == 0);

  chatbot_message_array_push (message_array, message1);
  g_assert_true (chatbot_message_array_length (message_array) == 1);
  g_assert_true (chatbot_message_array_at (message_array, 0) == message1);

  chatbot_message_array_push (message_array, message2);
  g_assert_true (chatbot_message_array_length (message_array) == 2);
  g_assert_true (chatbot_message_array_at (message_array, 0) == message1);
  g_assert_true (chatbot_message_array_at (message_array, 1) == message2);

  chatbot_message_array_push_take (message_array, message3);
  g_assert_true (chatbot_message_array_length (message_array) == 3);
  g_assert_true (chatbot_message_array_at (message_array, 0) == message1);
  g_assert_true (chatbot_message_array_at (message_array, 1) == message2);
  g_assert_true (chatbot_message_array_at (message_array, 2) == message3);

  message1_popped = chatbot_message_array_pop (message_array);
  g_assert_true (message1 == message1_popped);
  g_assert_true (chatbot_message_array_length (message_array) == 2);
  g_assert_true (chatbot_message_array_at (message_array, 0) == message2);
  g_assert_true (chatbot_message_array_at (message_array, 1) == message3);

  message2_popped = chatbot_message_array_pop (message_array);
  g_assert_true (message2 == message2_popped);
  g_assert_true (chatbot_message_array_length (message_array) == 1);
  g_assert_true (chatbot_message_array_at (message_array, 0) == message3);

  message3_popped = chatbot_message_array_pop (message_array);
  g_assert_true (message3 == message3_popped);
  g_assert_true (chatbot_message_array_length (message_array) == 0);
  g_assert_null (chatbot_message_array_pop (message_array));

  g_object_unref (message3_popped);
  g_object_unref (message2_popped);
  g_object_unref (message1_popped);
  g_object_unref (message2);
  g_object_unref (message1);
  g_object_unref (data);
  chatbot_message_array_free (message_array);
}

static JsonReader *
_json_parse (const gchar *json)
{
  JsonParser *parser = json_parser_new ();
  g_assert (json_parser_load_from_data (parser, json, -1, NULL));
  JsonReader *reader = json_reader_new (json_parser_get_root (parser));
  g_object_unref (parser);
  return reader;
}

int
main (int argc, char **argv)
{
  int ret;
  GVariant *variant;
  JsonReader *reader;

  g_test_init (&argc, &argv, NULL);
  chatbot_data_iface_register_type (CHATBOT_TEST_DATA_MIME_TYPE,
                                    CHATBOT_TYPE_TEST_DATA);
  variant = g_variant_parse (G_VARIANT_TYPE ("aa{sv}"),
                             "["
                             "{\"role\": <1>, \"mime-type\": "
                             "<\"text/plain\">, \"data\": <b\"SYSTEM\">}, "
                             "{\"role\": <2>, \"mime-type\": "
                             "<\"text/plain\">, \"data\": <b\"USER\">}, "
                             "{\"role\": <4>, \"mime-type\": "
                             "<\"text/plain\">, \"data\": <b\"ASSISTANT\">}"
                             "]",
                             NULL, NULL, NULL);
  g_assert (variant != NULL);
  reader = _json_parse (
      "{"
      "\"length\": 3, "
      "\"messages\": ["
      "{\"role\": 1, \"mime-type\": \"text/plain\", \"data\": \"SYSTEM\"}, "
      "{\"role\": 2, \"mime-type\": \"text/plain\", \"data\": \"USER\"}, "
      "{\"role\": 4, \"mime-type\": \"text/plain\", \"data\": \"ASSISTANT\"}"
      "]"
      "}");
  g_assert (reader != NULL);
  g_test_add_func ("/chatbot-message-array/ctor", test_ctor);
  g_test_add_data_func ("/chatbot-message-array/ctor/from_variant", variant,
                        test_ctor_from_variant);
  g_test_add_data_func ("/chatbot-message-array/ctor/from_json", reader,
                        test_ctor_from_json);
  g_test_add_func ("/chatbot-message-array/copy", test_copy);
  g_test_add_func ("/chatbot-message-array/to_variant", test_to_variant);
  g_test_add_func ("/chatbot-message-array/to_json", test_to_json);
  g_test_add_func ("/chatbot-message-array/push__at__length__pop",
                   test_push__at__length__pop);
  ret = g_test_run ();

  g_object_unref (reader);
  g_variant_unref (variant);
  return ret;
}
