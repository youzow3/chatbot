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

#include <test/chatbot/chatbot-test-language-model.h>

struct _ChatbotTestLanguageModel
{
  ChatbotModule parent;
};

static void chatbot_test_language_model_language_model_iface_init (
    ChatbotLanguageModelInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (
    ChatbotTestLanguageModel, chatbot_test_language_model, CHATBOT_TYPE_MODULE,
    G_IMPLEMENT_INTERFACE (
        CHATBOT_TYPE_LANGUAGE_MODEL,
        chatbot_test_language_model_language_model_iface_init));

static ChatbotMessageArray *
chatbot_test_language_model_generate (ChatbotLanguageModel *model,
                                      ChatbotMessageArray *messages,
                                      GCancellable *cancellable,
                                      GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_LANGUAGE_MODEL (model), NULL);
  g_return_val_if_fail (messages != NULL, NULL);
  g_return_val_if_fail (
      (cancellable == NULL) || G_IS_CANCELLABLE (cancellable), NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);

  return chatbot_message_array_copy (messages);
}

static gboolean
chatbot_test_language_model_save_state (ChatbotLanguageModel *model,
                                        const gchar *filename,
                                        GCancellable *cancellable,
                                        GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_LANGUAGE_MODEL (model), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (
      (cancellable == NULL) || G_IS_CANCELLABLE (cancellable), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  return TRUE;
}

static gboolean
chatbot_test_language_model_load_state (ChatbotLanguageModel *model,
                                        const gchar *filename,
                                        GCancellable *cancellable,
                                        GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_LANGUAGE_MODEL (model), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (
      (cancellable == NULL) || G_IS_CANCELLABLE (cancellable), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  return TRUE;
}

static void
chatbot_test_language_model_reset_state (ChatbotLanguageModel *model)
{
}

static ChatbotLanguageModel *
chatbot_test_language_model_fork (ChatbotLanguageModel *model)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_LANGUAGE_MODEL (model), NULL);
  g_object_ref (model);
  return model;
}

static void
chatbot_test_language_model_language_model_iface_init (
    ChatbotLanguageModelInterface *iface)
{
  iface->generate = chatbot_test_language_model_generate;
  iface->save_state = chatbot_test_language_model_save_state;
  iface->load_state = chatbot_test_language_model_load_state;
  iface->reset_state = chatbot_test_language_model_reset_state;
  iface->fork = chatbot_test_language_model_fork;
}

static const gchar *
chatbot_test_language_model_get_name (ChatbotModule *module)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_LANGUAGE_MODEL (module), NULL);
  return "test-language-model";
}

static const gchar *
chatbot_test_language_model_get_description (ChatbotModule *module)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_LANGUAGE_MODEL (module), NULL);
  return "test language model";
}

static void
chatbot_test_language_model_class_init (ChatbotTestLanguageModelClass *klass)
{
  ChatbotModuleClass *module_class = CHATBOT_MODULE_CLASS (klass);

  module_class->get_name = chatbot_test_language_model_get_name;
  module_class->get_description = chatbot_test_language_model_get_description;
}

static void
chatbot_test_language_model_init (ChatbotTestLanguageModel *model)
{
}
