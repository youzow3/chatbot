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

#define TEST_DATA "HELLO WORLD"

void
test_ctor (void)
{
  ChatbotTextPlain *text_plain = chatbot_text_plain_new (TEST_DATA);
  g_assert_nonnull (text_plain);
  g_object_unref (text_plain);
}

void
test_from_data (void)
{
  ChatbotData *from_data = chatbot_data_new_from_data (
      "text/plain", TEST_DATA, sizeof (TEST_DATA), NULL);
  g_assert_nonnull (from_data);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (from_data));
  g_object_unref (from_data);
}

void
test_from_text (void)
{
  ChatbotTextPlain *from_text
      = chatbot_data_new_from_text ("text/plain", TEST_DATA, NULL);
  g_assert_nonnull (from_text);
  g_assert_true (CHATBOT_IS_TEXT_PLAIN (from_text));
  g_object_unref (from_text);
}

void
test_get_data (void)
{
  gconstpointer data;
  gsize size;

  ChatbotTextPlain *text_plain = chatbot_text_plain_new (TEST_DATA);
  data = chatbot_data_get_data (CHATBOT_DATA (text_plain), &size);
  g_assert_true (size == sizeof (TEST_DATA));
  g_assert_true (g_str_equal (data, TEST_DATA));
  g_object_unref (text_plain);
}

void
test_get_text (void)
{
  const gchar *text;
  const gchar *text1;
  ChatbotTextPlain *text_plain = chatbot_text_plain_new (TEST_DATA);
  text = chatbot_data_get_text (CHATBOT_DATA (text_plain));
  g_assert_true (g_str_equal (text, TEST_DATA));
  text1 = chatbot_text_plain_get_text (text_plain);
  g_assert_true (text == text1);
  g_object_unref (text_plain);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/chatbot-text-plain/ctor", test_ctor);
  g_test_add_func ("/chatbot-text-plain/from_data", test_from_data);
  g_test_add_func ("/chatbot-text-plain/from_text", test_from_text);
  g_test_add_func ("/chatbot-text-plain/get_data", test_get_data);
  g_test_add_func ("/chatbot-text-plain/get_text", test_get_text);
  return g_test_run ();
}
