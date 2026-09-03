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
#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "chatbot-message-array.h"
#include "chatbot-module.h"

G_BEGIN_DECLS

#define CHATBOT_TYPE_LANGUAGE_MODEL chatbot_language_model_get_type ()
G_DECLARE_INTERFACE (ChatbotLanguageModel, chatbot_language_model, CHATBOT,
                     LANGUAGE_MODEL, ChatbotModule);

struct _ChatbotLanguageModelInterface
{
  GTypeInterface iface;

  ChatbotMessageArray *(*generate) (ChatbotLanguageModel *language_model,
                                    ChatbotMessageArray *messages,
                                    GCancellable *cancellable, GError **error);
  gboolean (*save_state) (ChatbotLanguageModel *language_model,
                          const gchar *filename, GCancellable *cancellable,
                          GError **error);
  gboolean (*load_state) (ChatbotLanguageModel *language_model,
                          const gchar *filename, GCancellable *cancellable,
                          GError **error);
  void (*reset_state) (ChatbotLanguageModel *language_model);
  ChatbotLanguageModel *(*fork) (ChatbotLanguageModel *language_model);
};

gpointer chatbot_language_model_new (GType type, const gchar *parameter,
                                     GError **error);
ChatbotMessageArray *
chatbot_language_model_generate (ChatbotLanguageModel *language_model,
                                 ChatbotMessageArray *messages,
                                 GCancellable *cancellable, GError **error);
void chatbot_language_model_generate_async (
    ChatbotLanguageModel *language_model, ChatbotMessageArray *messages,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);
ChatbotMessageArray *
chatbot_language_model_generate_finish (ChatbotLanguageModel *language_model,
                                        GAsyncResult *result, GError **error);
gboolean
chatbot_language_model_save_state (ChatbotLanguageModel *language_model,
                                   const gchar *filename,
                                   GCancellable *cancellable, GError **error);
void chatbot_language_model_save_state_async (
    ChatbotLanguageModel *language_model, const gchar *filename,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);
gboolean
chatbot_language_model_save_state_finish (ChatbotLanguageModel *language_model,
                                          GAsyncResult *result,
                                          GError **error);
gboolean
chatbot_language_model_load_state (ChatbotLanguageModel *language_model,
                                   const gchar *filename,
                                   GCancellable *cancellable, GError **error);
void chatbot_language_model_load_state_async (
    ChatbotLanguageModel *language_model, const gchar *filename,
    GCancellable *cancellable, GAsyncReadyCallback callback,
    gpointer user_data);
gboolean
chatbot_language_model_load_state_finish (ChatbotLanguageModel *language_model,
                                          GAsyncResult *result,
                                          GError **error);
void chatbot_language_model_reset_state (ChatbotLanguageModel *language_model);
ChatbotLanguageModel *
chatbot_language_model_fork (ChatbotLanguageModel *language_model);

G_END_DECLS
