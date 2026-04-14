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

/**
 * chatbot_trainer_new:
 * @type: GType of class that implement [iface@Trainer].
 * @parameter: Construction parameter for the @type.
 * @error: (out) (optional): Location to store error.
 *
 * Construct subclass of [iface@Trainer].
 *
 * Returns: (transfer full): Newly constructed instance.
 */
gpointer
chatbot_trainer_new (GType type, const gchar *parameter, GError **error)
{
  return g_initable_new (type, NULL, error, "raw_parameter", parameter, NULL);
}

/**
 * chatbot_trainer_train:
 * @trainer: instance
 * @data: (array length=data_len): Training data array
 * @data_len: Length of @data
 * @cancellable: (nullable): %GCancellable instance
 * @error: (nullable): Location to store runtime errors
 *
 * Trains model with given training data.
 *
 * Trains model with given training data. This function will ignore data which
 * does not match expected type(s). Implementer will handle all configuration
 * of training. This function should also perform saving trained weight for
 * inference.
 *
 * Runtime error should only be happened for actual training logic, not data
 * length or data types mismatch.
 *
 * Returns: %TRUE if training is successfully finished and %FALSE if something
 * went wrong.
 */
gboolean
chatbot_trainer_train (ChatbotTrainer *trainer, ChatbotData **data,
                       size_t data_len, GCancellable *cancellable,
                       GError **error)
{
  ChatbotTrainerInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TRAINER (trainer), FALSE);
  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (G_IS_CANCELLABLE (cancellable) || cancellable == NULL,
                        FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);
  iface = CHATBOT_TRAINER_GET_IFACE (trainer);
  if (iface->train == NULL)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                   "Trainer implementation doesn't provide train().");
      return FALSE;
    }
  return iface->train (trainer, data, data_len, cancellable, error);
}
