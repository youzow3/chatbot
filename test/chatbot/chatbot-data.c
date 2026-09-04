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

typedef struct
{
  void *none;
} NoFixture;

static void
test_registered_setup (NoFixture *fixture, gconstpointer user_data)
{
  chatbot_data_iface_register_type (CHATBOT_TEST_DATA_MIME_TYPE,
                                    CHATBOT_TYPE_TEST_DATA);
}

static void
test_registered_teardown (NoFixture *fixture, gconstpointer user_data)
{
  chatbot_data_iface_unregister_type (CHATBOT_TEST_DATA_MIME_TYPE);
}

static void
test_ctor_from_data_registered (NoFixture *fixture, gconstpointer user_data)
{
  GError *error = NULL;
  ChatbotData *data = NULL;

  data = chatbot_data_new_from_data (CHATBOT_TEST_DATA_MIME_TYPE,
                                     CHATBOT_TEST_DATA_DATA,
                                     sizeof (CHATBOT_TEST_DATA_DATA), &error);
  g_assert_nonnull (data);
  g_assert_no_error (error);
  g_object_unref (data);
}

static void
test_ctor_from_data_not_registered (void)
{
  GError *error = NULL;
  ChatbotData *data = NULL;

  data = chatbot_data_new_from_data (CHATBOT_TEST_DATA_MIME_TYPE,
                                     CHATBOT_TEST_DATA_DATA,
                                     sizeof (CHATBOT_TEST_DATA_DATA), &error);
  g_assert_null (data);
  g_assert_error (error, CHATBOT_ERROR, CHATBOT_ERROR_NOT_FOUND);
}

static void
test_ctor_from_text_registered (NoFixture *fixture, gconstpointer user_data)
{
  GError *error = NULL;
  ChatbotData *data = NULL;

  data = chatbot_data_new_from_text (CHATBOT_TEST_DATA_MIME_TYPE,
                                     CHATBOT_TEST_DATA_DATA, &error);
  g_assert_nonnull (data);
  g_assert_no_error (error);
  g_object_unref (data);
}

static void
test_ctor_from_text_not_registered (void)
{
  GError *error = NULL;
  ChatbotData *data = NULL;

  data = chatbot_data_new_from_text (CHATBOT_TEST_DATA_MIME_TYPE,
                                     CHATBOT_TEST_DATA_DATA, &error);
  g_assert_null (data);
  g_assert_error (error, CHATBOT_ERROR, CHATBOT_ERROR_NOT_FOUND);
}

static void
test_get_data (NoFixture *fixture, gconstpointer user_data)
{
  ChatbotData *data = chatbot_data_new_from_data (
      CHATBOT_TEST_DATA_MIME_TYPE, CHATBOT_TEST_DATA_DATA,
      sizeof (CHATBOT_TEST_DATA_DATA), NULL);
  gsize size;
  gconstpointer data_ptr = chatbot_data_get_data (data, &size);

  g_assert_true (g_str_equal (data_ptr, CHATBOT_TEST_DATA_DATA));
  g_assert_true (size == sizeof (CHATBOT_TEST_DATA_DATA));
  g_object_unref (data);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);
  g_test_add ("/chatbot-data/ctor/from_data/registered", NoFixture, NULL,
              test_registered_setup, test_ctor_from_data_registered,
              test_registered_teardown);
  g_test_add_func ("/chatbot-data/ctor/from_data/not-registered",
                   test_ctor_from_data_not_registered);
  g_test_add ("/chatbot-data/ctor/from_text/registered", NoFixture, NULL,
              test_registered_setup, test_ctor_from_text_registered,
              test_registered_teardown);
  g_test_add_func ("/chatbot-data/ctor/from_text/not-registered",
                   test_ctor_from_text_not_registered);
  g_test_add ("/chatbot-data/get_data", NoFixture, NULL, test_registered_setup,
              test_get_data, test_registered_teardown);
  return g_test_run ();
}
