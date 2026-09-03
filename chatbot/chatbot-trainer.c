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
 * Interface to define training facilities.
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
 * Constructs an instance of [iface@Chatbot.Trainer].
 *
 * Returns: (transfer full): Newly constructed instance.
 */
gpointer
chatbot_trainer_new (GType type, const gchar *parameter, GError **error)
{
  g_return_val_if_fail (g_type_is_a (type, CHATBOT_TYPE_TRAINER), NULL);
  return g_initable_new (type, NULL, error, "raw_parameter", parameter, NULL);
}

/**
 * chatbot_trainer_train:
 * @trainer: instance
 * @data: (array length=data_len): Training data array
 * @data_len: Length of @data
 * @cancellable: (nullable): #GCancellable instance
 * @error: (out) (optional): Location to store runtime errors
 *
 * Trains model with given training data.
 *
 * This method should do all procedure include:
 *
 * 1. Initializing/Loading weight data.
 *
 * 2. Training.
 *
 * 3. Saving trained weight data.
 *
 * This method must skip invalid data for training, and so, runtime error
 * related @data should not be throwned.
 *
 * If a class implements both this and [iface@Chatbot.LanguageModel],
 * [method@Chatbot.Trainer.train] should update weights used in
 * [iface@Chatbot.LanguageModel] too.
 *
 * Returns: %TRUE if training is successfully finished and %FALSE if something
 * went wrong.
 */
gboolean
chatbot_trainer_train (ChatbotTrainer *trainer, ChatbotMessageArray **data,
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

typedef struct
{
  ChatbotMessageArray **data;
  gsize data_len;
} _TRAIN_THREAD_DATA;

_TRAIN_THREAD_DATA *
train_thread_data_new (ChatbotMessageArray **data, gsize data_len)
{
  _TRAIN_THREAD_DATA *th_data;

  th_data = g_new (_TRAIN_THREAD_DATA, 1);
  th_data->data = g_new (ChatbotMessageArray *, data_len);
  for (gsize i = 0; i < data_len; i++)
    th_data->data[i] = chatbot_message_array_copy (data[i]);
  th_data->data_len = data_len;
  return th_data;
}

void
train_thread_data_free (_TRAIN_THREAD_DATA *th_data)
{
  for (gsize i = 0; i < th_data->data_len; i++)
    chatbot_message_array_free (th_data->data[i]);
  g_free (th_data->data);
  g_free (th_data);
}

static void
_train_thread (GTask *task, gpointer source_object, gpointer task_data,
               GCancellable *cancellable)
{
  ChatbotTrainer *trainer;
  _TRAIN_THREAD_DATA *th_data;
  GError *error = NULL;

  g_return_if_fail (G_IS_TASK (task));
  g_return_if_fail (CHATBOT_IS_TRAINER (source_object));
  g_return_if_fail (task_data != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));
  trainer = CHATBOT_TRAINER (source_object);
  th_data = (_TRAIN_THREAD_DATA *)task_data;

  if (!chatbot_trainer_train (trainer, th_data->data, th_data->data_len,
                              cancellable, &error))
    {
      g_task_return_error (task, error);
      return;
    }
  g_task_return_boolean (task, TRUE);
}

/**
 * chatbot_trainer_train_async:
 * @trainer: instance
 * @data: (array length=data_len): Training data array
 * @data_len: Length of @data
 * @cancellable: (nullable): #GCancellable instance
 * @callback: (nullable): A #GAsyncReadyCallback.
 * @user_data: (nullable): Data passed to @callback.
 *
 * Asynchronous version of [method@Chatbot.Trainer.train].
 */
void
chatbot_trainer_train_async (ChatbotTrainer *trainer,
                             ChatbotMessageArray **data, gsize data_len,
                             GCancellable *cancellable,
                             GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task;

  g_return_if_fail (CHATBOT_IS_TRAINER (trainer));
  g_return_if_fail (data != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (trainer, cancellable, callback, user_data);
  g_task_set_task_data (task, train_thread_data_new (data, data_len),
                        (GDestroyNotify)train_thread_data_free);
  g_task_run_in_thread (task, _train_thread);
  g_object_unref (task);
}

/**
 * chatbot_trainer_train_finish:
 * @trainer: self
 * @result: A #GAsyncResult.
 * @error: (out) (optional): Location to store runtime error.
 *
 * Finishes training started with [method@Chatbot.Trainer.train].
 */
gboolean
chatbot_trainer_train_finish (ChatbotTrainer *trainer, GAsyncResult *result,
                              GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_TRAINER (trainer), FALSE);
  g_return_val_if_fail (
      G_IS_ASYNC_RESULT (result)
          && (G_OBJECT (trainer) == g_async_result_get_source_object (result)),
      FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}
