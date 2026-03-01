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
 * ChatbotTrainer:
 *
 * Interface to implement training of AI models.
 *
 * There are two ways to implement Trainer for models.
 *
 * 1. Implement Trainer to the class which also implements
 * [iface@LanguageModel].
 *
 * 2. Implement Trainer and [iface@LanguageModel] (or other model class)
 * separated classes.
 *
 * Option 1 is better for model implementation can be re-used to model
 * training. Option 2 is better for model implementation and model training
 * implementation are completely separated.
 */

#include "chatbot-trainer.h"

G_DEFINE_INTERFACE (ChatbotTrainer, chatbot_trainer, CHATBOT_TYPE_MODULE);

static void
chatbot_trainer_default_init (ChatbotTrainerInterface *iface)
{
}

gboolean
chatbot_trainer_train (ChatbotTrainer *trainer, GValue **data, gsize len,
                       GCancellable *cancellable, GError **error)
{
  ChatbotTrainerInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TRAINER (trainer), FALSE);
  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (cancellable != NULL, FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);
  iface = CHATBOT_TRAINER_GET_IFACE (trainer);
  if (iface->train == NULL)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                   "Trainer implementation doesn't provide train().");
      return FALSE;
    }
  return iface->train (trainer, data, len, cancellable, error);
}
