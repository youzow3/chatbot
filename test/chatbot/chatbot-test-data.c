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

#include <test/chatbot/chatbot-test-error.h>

#include <test/chatbot/chatbot-test-data.h>

static void chatbot_data_iface_init (ChatbotDataInterface *iface);

struct _ChatbotTestData
{
  GObject parent;
};

G_DEFINE_FINAL_TYPE_WITH_CODE (
    ChatbotTestData, chatbot_test_data, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (CHATBOT_TYPE_DATA, chatbot_data_iface_init));

static gboolean
chatbot_test_data_init_from_data (ChatbotData *data, gconstpointer raw_data,
                                  gsize size, GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_DATA (data), FALSE);
  g_return_val_if_fail (raw_data != NULL, FALSE);
  g_return_val_if_fail (size == sizeof (CHATBOT_TEST_DATA_DATA), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  if (strncmp (raw_data, CHATBOT_TEST_DATA_DATA,
               strlen (CHATBOT_TEST_DATA_DATA)))
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Invalid data");
      return FALSE;
    }

  return TRUE;
}

static gboolean
chatbot_test_data_init_from_text (ChatbotData *data, const gchar *text,
                                  GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_DATA (data), FALSE);
  g_return_val_if_fail (text != NULL, FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  if (strcmp (text, CHATBOT_TEST_DATA_DATA))
    {
      g_set_error (error, CHATBOT_ERROR, CHATBOT_ERROR_INVALID,
                   "Invalid data");
      return FALSE;
    }

  return TRUE;
}

static gconstpointer
chatbot_test_data_get_data (ChatbotData *data, gsize *size)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_DATA (data), NULL);
  if (size != NULL)
    *size = sizeof (CHATBOT_TEST_DATA_DATA);
  return CHATBOT_TEST_DATA_DATA;
}

static const gchar *
chatbot_test_data_get_text (ChatbotData *data)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_DATA (data), NULL);
  return CHATBOT_TEST_DATA_DATA;
}

static void
chatbot_data_iface_init (ChatbotDataInterface *iface)
{
  iface->init_from_data = chatbot_test_data_init_from_data;
  iface->init_from_text = chatbot_test_data_init_from_text;
  iface->get_data = chatbot_test_data_get_data;
  iface->get_text = chatbot_test_data_get_text;
}

static void
chatbot_test_data_class_init (ChatbotTestDataClass *klass)
{
}

static void
chatbot_test_data_init (ChatbotTestData *mock_data)
{
}

ChatbotTestData *
chatbot_test_data_new (void)
{
  return g_object_new (CHATBOT_TYPE_TEST_DATA, NULL);
}
