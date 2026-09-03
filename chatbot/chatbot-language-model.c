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
 * ChatbotLanguageModel:
 *
 * Interface to define language model.
 *
 * This interface acts like single user chatbot.
 *
 * [method@Chatbot.LanguageModel.generate] generates model response and update
 * internal state like its KV-cache. To copy state and continue another chat
 * session, use [method@Chatbot.LanguageModel.fork].
 *
 * [method@Chatbot.LanguageModel.save_state] and
 * [method@Chatbot.LanguageModel.load_state] is used to persist model state to
 * storage.
 *
 * [method@Chatbot.LanguageModel.reset_state] reset model state.
 */

#include "chatbot-language-model.h"

enum
{
  GENERATING,
  COMPLETED,
  N_SIGNALS
};

static int signals[N_SIGNALS];

G_DEFINE_INTERFACE (ChatbotLanguageModel, chatbot_language_model,
                    CHATBOT_TYPE_MODULE);

static void
chatbot_language_model_default_init (ChatbotLanguageModelInterface *iface)
{
  /**
   * ChatbotLanguageModel::generating:
   * @language_model: language model instance
   * @message: Incomplete message
   *
   * Emits when a token is generated.
   *
   * This signal is emitted if the message is generated and its partial data is
   * (partially) valid.
   *
   * This signal is mainly for streaming model response.
   */
  signals[GENERATING] = g_signal_new (
      "generating", CHATBOT_TYPE_LANGUAGE_MODEL, G_SIGNAL_RUN_LAST, 0, NULL,
      NULL, NULL, G_TYPE_NONE, 1, CHATBOT_TYPE_MESSAGE);

  /**
   * ChatbotLanguageModel::completed:
   * @language_model: language model instance
   * @message: generated message
   *
   * Emits when a message generation is completed.
   *
   * This signal is emitted when one of a [class@Chatbot.Message] that will be
   * returned by [method@Chatbot.LanguageModel.generate] is completely
   * generated.
   */
  signals[COMPLETED] = g_signal_new ("completed", CHATBOT_TYPE_LANGUAGE_MODEL,
                                     G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
                                     G_TYPE_NONE, 1, CHATBOT_TYPE_MESSAGE);
}

/**
 * chatbot_language_model_new:
 * @type: GType of class that implement [iface@LanguageModel].
 * @parameter: Construction parameter for the @type.
 * @error: (out) (optional): Location to store error.
 *
 * Constructs an instance.
 *
 * Returns: (transfer full): Newly constructed instance.
 */
gpointer
chatbot_language_model_new (GType type, const gchar *parameter, GError **error)
{
  g_return_val_if_fail (g_type_is_a (type, CHATBOT_TYPE_LANGUAGE_MODEL), NULL);
  return g_initable_new (type, NULL, error, "raw_parameter", parameter, NULL);
}

/**
 * chatbot_language_model_generate:
 * @language_model: language model
 * @messages: messages.
 * @cancellable: (nullable): #GCancellable to cancel operation or %NULL.
 * @error: (out) (optional): Location to store error or %NULL.
 *
 * Generates until stop condition is met.
 *
 * The stop condition depends on the implementation.
 *
 * Returns: (nullable) (transfer full): Generated message(s) or
 * %NULL on failure.
 */
ChatbotMessageArray *
chatbot_language_model_generate (ChatbotLanguageModel *language_model,
                                 ChatbotMessageArray *messages,
                                 GCancellable *cancellable, GError **error)
{
  ChatbotLanguageModelInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model), FALSE);
  g_return_val_if_fail (messages != NULL, FALSE);
  g_return_val_if_fail (
      (cancellable == NULL) || G_IS_CANCELLABLE (cancellable), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  iface = CHATBOT_LANGUAGE_MODEL_GET_IFACE (language_model);
  g_return_val_if_fail (iface->generate != NULL, FALSE);
  return iface->generate (language_model, messages, cancellable, error);
}

static void
_generate_thread (GTask *task, gpointer source_object, gpointer task_data,
                  GCancellable *cancellable)
{
  ChatbotMessageArray *result;
  GError *error = NULL;

  g_return_if_fail (G_IS_TASK (task));
  g_return_if_fail (CHATBOT_IS_LANGUAGE_MODEL (source_object));
  g_return_if_fail (task_data != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  result = chatbot_language_model_generate (
      CHATBOT_LANGUAGE_MODEL (source_object), (ChatbotMessageArray *)task_data,
      cancellable, &error);
  if (result == NULL)
    {
      g_task_return_error (task, error);
      return;
    }
  g_task_return_pointer (task, result, NULL);
}

/**
 *  chatbot_language_model_generate_async:
 *  @language_model: language model
 *  @messages: messages.
 *  @cancellable: (nullable): #GCancellable object or %NULL.
 *  @callback: (nullable): A #AsyncReadyCallback.
 *  @user_data: (nullable): Data passed to @callback.
 *
 *  Asynchronous version of [method@Chatbot.LanguageModel.generate].
 */
void
chatbot_language_model_generate_async (ChatbotLanguageModel *language_model,
                                       ChatbotMessageArray *messages,
                                       GCancellable *cancellable,
                                       GAsyncReadyCallback callback,
                                       gpointer user_data)
{
  GTask *task;

  g_return_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model));
  g_return_if_fail (messages != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (language_model, cancellable, callback, user_data);
  g_task_set_task_data (task, chatbot_message_array_copy (messages),
                        (GDestroyNotify)chatbot_message_array_free);
  g_task_run_in_thread (task, _generate_thread);
  g_object_unref (task);
}

/**
 * chatbot_language_model_generate_finish:
 * @language_model: language model
 * @result: A #GAsyncResult.
 * @error: (out) (nullable) (optional): Location to store error or %NULL.
 *
 * Finishes an asynchronous generation started with
 * [method@Chatbot.LanguageModel.generate_async].
 *
 * Returns: (transfer full): Generated messages.
 */
ChatbotMessageArray *
chatbot_language_model_generate_finish (ChatbotLanguageModel *language_model,
                                        GAsyncResult *result, GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model), NULL);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result)
                            && (G_OBJECT (language_model)
                                == g_async_result_get_source_object (result)),
                        NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

/**
 * chatbot_language_model_save_state:
 * @filename: File or directory path to save state.
 * @cancellable: (nullable): #GCancellable to cancel operation.
 * @error: (out) (optional): Location to store error.
 *
 * Saves state to specified file or directory.
 *
 * Returns: %TRUE if succeed, %FALSE on failure.
 */
gboolean
chatbot_language_model_save_state (ChatbotLanguageModel *language_model,
                                   const gchar *filename,
                                   GCancellable *cancellable, GError **error)
{
  ChatbotLanguageModelInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model), FALSE);
  g_return_val_if_fail (filename, FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  iface = CHATBOT_LANGUAGE_MODEL_GET_IFACE (language_model);
  if (iface->save_state == NULL)
    {
      // TODO check using G_IO_ERROR is OK or need to prepare own error
      // namespace.
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                   "Saving state is not supported for this module.");
      return FALSE;
    }

  return iface->save_state (language_model, filename, cancellable, error);
}

static void
_save_state_thread (GTask *task, gpointer source_object, gpointer task_data,
                    GCancellable *cancellable)
{
  GError *error = NULL;

  g_return_if_fail (G_IS_TASK (task));
  g_return_if_fail (CHATBOT_IS_LANGUAGE_MODEL (source_object));
  g_return_if_fail (task_data != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  if (!chatbot_language_model_save_state (
          CHATBOT_LANGUAGE_MODEL (source_object), (const gchar *)task_data,
          cancellable, &error))
    g_task_return_error (task, error);
  g_task_return_boolean (task, TRUE);
}

/**
 * chatbot_language_model_save_state_async:
 * @language_model: language model
 * @filename: File or directory path to save state.
 * @cancellable: (nullable): A #GCancellable object or %NULL.
 * @callback: (nullable): A #GAsyncReadyCallback.
 * @user_data: (nullable): A data passed to @callback.
 *
 * Asynchronous version of [method@Chatbot.LanguageModel.save_state].
 */
void
chatbot_language_model_save_state_async (ChatbotLanguageModel *language_model,
                                         const gchar *filename,
                                         GCancellable *cancellable,
                                         GAsyncReadyCallback callback,
                                         gpointer user_data)
{
  GTask *task;

  g_return_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model));
  g_return_if_fail (filename != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (language_model, cancellable, callback, user_data);
  g_task_set_task_data (task, g_strdup (filename), g_free);
  g_task_run_in_thread (task, _save_state_thread);
  g_object_unref (task);
}

/**
 * chatbot_language_model_save_state_finish:
 * @language_model: language model
 * @result: A #GAsyncResult.
 * @error: (out) (optional) (nullable): Location to store error.
 *
 * Finishes operation started with [method@Chatbot.LanguageModel.save_state].
 *
 * Returns: %TRUE if the state is saved successfully, and %FALSE if it is
 * failed.
 */
gboolean
chatbot_language_model_save_state_finish (ChatbotLanguageModel *language_model,
                                          GAsyncResult *result, GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model), FALSE);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result)
                            && (G_OBJECT (language_model)
                                == g_async_result_get_source_object (result)),
                        FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}

/**
 * chatbot_language_model_load_state:
 * @filename: File or directory path to load state.
 * @cancellable: (nullable): #GCancellable to cancel operation.
 * @error: (out) (optional): Location to store error.
 *
 * Loads state from specified file.
 *
 * Returns: %TRUE if succeed, %FALSE on failure.
 */
gboolean
chatbot_language_model_load_state (ChatbotLanguageModel *language_model,
                                   const gchar *filename,
                                   GCancellable *cancellable, GError **error)
{
  ChatbotLanguageModelInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model), FALSE);
  g_return_val_if_fail (filename, FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  iface = CHATBOT_LANGUAGE_MODEL_GET_IFACE (language_model);
  if (iface->load_state == NULL)
    {
      // TODO check using G_IO_ERROR is OK or need to prepare own error
      // namespace.
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                   "Saving state is not supported for this module.");
      return FALSE;
    }

  return iface->load_state (language_model, filename, cancellable, error);
}

static void
_load_state_thread (GTask *task, gpointer source_object, gpointer task_data,
                    GCancellable *cancellable)
{
  GError *error = NULL;

  g_return_if_fail (G_IS_TASK (task));
  g_return_if_fail (CHATBOT_IS_LANGUAGE_MODEL (source_object));
  g_return_if_fail (task_data != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  if (!chatbot_language_model_load_state (
          CHATBOT_LANGUAGE_MODEL (source_object), (const gchar *)task_data,
          cancellable, &error))
    {
      g_task_return_error (task, error);
      return;
    }
  g_task_return_boolean (task, TRUE);
}

/**
 * chatbot_language_model_load_state_async:
 * @language_model: language model
 * @filename: File or directory path to load state.
 * @cancellable: (nullable): #GCancellable to cancel operation.
 * @callback: (nullable): A #GAsyncReadyCallback.
 * @user_data: (nullable): Data passed to @callback.
 *
 * Asynchronous version of [method@Chatbot.LanguageModel.load_state].
 */
void
chatbot_language_model_load_state_async (ChatbotLanguageModel *language_model,
                                         const gchar *filename,
                                         GCancellable *cancellable,
                                         GAsyncReadyCallback callback,
                                         gpointer user_data)
{
  GTask *task;

  g_return_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model));
  g_return_if_fail (filename != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (language_model, cancellable, callback, user_data);
  g_task_set_task_data (task, g_strdup (filename), g_free);
  g_task_run_in_thread (task, _load_state_thread);
  g_object_unref (task);
}

/**
 * chatbot_language_model_load_state_finish:
 * @language_model: language model
 * @result: A #GAsyncResult.
 * @error: (out) (optional) (nullable): Location to store the error.
 *
 * Finishes operation started with
 * [method@Chatbot.LanguageModel.load_state_async].
 *
 * Returns: %TRUE if the state is loaded successfully, and %FALSE if it is
 * failed to load.
 */
gboolean
chatbot_language_model_load_state_finish (ChatbotLanguageModel *language_model,
                                          GAsyncResult *result, GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model), FALSE);
  g_return_val_if_fail (G_IS_ASYNC_RESULT (result)
                            && (G_OBJECT (language_model)
                                == g_async_result_get_source_object (result)),
                        FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}

/**
 * chatbot_language_model_reset_state:
 *
 * Resets model state.
 *
 * Resetting model state means clean-up existing KV-cache, or reset state
 * matrix to initial value.
 */
void
chatbot_language_model_reset_state (ChatbotLanguageModel *language_model)
{
  ChatbotLanguageModelInterface *iface;
  g_return_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model));

  iface = CHATBOT_LANGUAGE_MODEL_GET_IFACE (language_model);
  if (iface->reset_state == NULL)
    return;
  iface->reset_state (language_model);
}

/**
 * chatbot_language_model_fork:
 * @language_model: language model
 *
 * Forks instance.
 *
 * @language_model and forked instance should have same state, and it must be
 * copied instead of shared by ref counting.
 *
 * Returns: (transfer full): Forked instance.
 */
ChatbotLanguageModel *
chatbot_language_model_fork (ChatbotLanguageModel *language_model)
{
  ChatbotLanguageModelInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model), FALSE);

  iface = CHATBOT_LANGUAGE_MODEL_GET_IFACE (language_model);
  g_return_val_if_fail (iface->fork != NULL, FALSE);
  return iface->fork (language_model);
}
