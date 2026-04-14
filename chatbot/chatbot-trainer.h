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

#include "chatbot-data.h"
#include "chatbot-module.h"

G_BEGIN_DECLS

#define CHATBOT_TYPE_TRAINER chatbot_trainer_get_type ()
G_DECLARE_INTERFACE (ChatbotTrainer, chatbot_trainer, CHATBOT, TRAINER,
                     ChatbotModule);

struct _ChatbotTrainerInterface
{
  GTypeInterface iface;
  gboolean (*train) (ChatbotTrainer *trainer, ChatbotData **data,
                     size_t data_len, GCancellable *cancellable,
                     GError **error);
};

gpointer chatbot_trainer_new (GType type, const gchar *parameter,
                              GError **error);
gboolean chatbot_trainer_train (ChatbotTrainer *trainer, ChatbotData **data,
                                size_t data_len, GCancellable *cancellable,
                                GError **error);

G_END_DECLS
