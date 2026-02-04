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

#include "chatbot-module.h"

G_BEGIN_DECLS

#define CHATBOT_TYPE_LANGUAGE_MODEL chatbot_language_model_get_type ()
G_DECLARE_INTERFACE (ChatbotLanguageModel, chatbot_language_model, CHATBOT,
                     LANGUAGE_MODEL, ChatbotModule);

struct _ChatbotLanguageModelInterface
{
  GTypeInterface iface;

  gchar *(*apply_chat_template) (ChatbotLanguageModel *language_model,
                                 const GStrv role_and_message);
  gboolean (*prefill) (ChatbotLanguageModel *language_model, const gchar *text,
                       GError **error);
  gchar *(*generate) (ChatbotLanguageModel *language_model, GError **error);
  gboolean (*save_state) (ChatbotLanguageModel *language_model,
                          const gchar *filename, GError **error);
  gboolean (*load_state) (ChatbotLanguageModel *language_model,
                          const gchar *filename, GError **error);
};

gpointer chatbot_language_model_new (GType type, const gchar *parameter,
                                     GError **error);
gchar *chatbot_language_model_apply_chat_template (
    ChatbotLanguageModel *language_model, const GStrv role_and_message);
gboolean chatbot_language_model_prefill (ChatbotLanguageModel *language_model,
                                         const gchar *text, GError **error);
gchar *chatbot_language_model_generate (ChatbotLanguageModel *language_model,
                                        GError **error);
gboolean
chatbot_language_model_save_state (ChatbotLanguageModel *language_model,
                                   const gchar *filename, GError **error);
gboolean
chatbot_language_model_load_state (ChatbotLanguageModel *language_model,
                                   const gchar *filename, GError **error);

G_END_DECLS
