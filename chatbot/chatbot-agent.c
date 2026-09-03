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
 * ChatbotAgent:
 *
 * Interface to define agent for agentic system.
 *
 * This library uses agent-centric abstraction, so this interface is where
 * agentic loop is implemented.
 */

#include <chatbot/chatbot-message.h>

#include <chatbot/chatbot-agent.h>

enum
{
  GENERATING,
  COMPLETED,
  N_SIGNALS
};

static guint signals[N_SIGNALS];

G_DEFINE_INTERFACE (ChatbotAgent, chatbot_agent, CHATBOT_TYPE_MODULE);

static void
chatbot_agent_default_init (ChatbotAgentInterface *iface)
{
  /**
   * ChatbotAgent::generating:
   * @agent: language model instance
   * @message: Incomplete message
   *
   * Emits when a token is generated.
   *
   * This signal is emitted if the message is generated and its partial data is
   * (partially) valid.
   *
   * This signal is mainly for streaming model response.
   */
  signals[GENERATING]
      = g_signal_new ("generating", CHATBOT_TYPE_AGENT, G_SIGNAL_RUN_LAST, 0,
                      NULL, NULL, NULL, G_TYPE_NONE, 1, CHATBOT_TYPE_MESSAGE);

  /**
   * ChatbotAgent::completed:
   * @agent: language model instance
   * @message: generated message
   *
   * Emits when a message generation is completed.
   *
   * This signal is emitted when one of a [class@Chatbot.Message] that will be
   * returned by [method@Chatbot.Agent.act] is completely generated.
   */
  signals[COMPLETED]
      = g_signal_new ("completed", CHATBOT_TYPE_AGENT, G_SIGNAL_RUN_LAST, 0,
                      NULL, NULL, NULL, G_TYPE_NONE, 1, CHATBOT_TYPE_MESSAGE);
}

/**
 * chatbot_agent_act:
 * @agent: Agent.
 * @messages: Newly generated or retrieved messages
 * @cancellable: (nullable): %GCancellable object or %NULL.
 * @error: (out) (optional) : Location to store error.
 *
 * Performs agentic loop according to given @messages.
 *
 * The last element of returned messages should have
 * [enum@Chatbot.MessageRole.ASSISTANT] with `text/plain` data.
 *
 * Returns: Messages agent has generated.
 */
ChatbotMessageArray *
chatbot_agent_act (ChatbotAgent *agent, ChatbotMessageArray *messages,
                   GCancellable *cancellable, GError **error)
{
  ChatbotAgentInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_AGENT (agent), NULL);
  g_return_val_if_fail (messages != NULL, NULL);
  g_return_val_if_fail (
      (cancellable == NULL) || G_IS_CANCELLABLE (cancellable), NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);

  iface = CHATBOT_AGENT_GET_IFACE (agent);
  g_return_val_if_fail (iface->act != NULL, NULL);
  return iface->act (agent, messages, cancellable, error);
}

static void
_act_thread (GTask *task, gpointer source_object, gpointer task_data,
             GCancellable *cancellable)
{
  ChatbotMessageArray *messages;
  GError *error = NULL;

  g_return_if_fail (G_IS_TASK (task));
  g_return_if_fail (CHATBOT_IS_AGENT (source_object));
  g_return_if_fail (task_data != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  messages = chatbot_agent_act (CHATBOT_AGENT (source_object),
                                (ChatbotMessageArray *)task_data, cancellable,
                                &error);
  if (messages == NULL)
    {
      g_task_return_error (task, error);
      return;
    }
  g_task_return_pointer (task, messages, NULL);
}

/**
 * chatbot_agent_act_async:
 * @agent: Agent.
 * @messages: Newly generated or retrieved messages
 * @cancellable: (nullable): %GCancellable object or %NULL.
 * @callback: (nullable): A %AsyncReadyCallback.
 * @user_data: (nullable): Data passed to @callback.
 *
 * Asynchornous version of [method@Chatbot.Agent.act].
 */
void
chatbot_agent_act_async (ChatbotAgent *agent, ChatbotMessageArray *messages,
                         GCancellable *cancellable,
                         GAsyncReadyCallback callback, gpointer user_data)
{
  ChatbotAgentInterface *iface;
  GTask *task;

  g_return_if_fail (CHATBOT_IS_AGENT (agent));
  g_return_if_fail (messages != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  iface = CHATBOT_AGENT_GET_IFACE (agent);
  g_return_if_fail (iface->act != NULL);
  task = g_task_new (agent, cancellable, callback, user_data);
  g_task_set_task_data (task, chatbot_message_array_copy (messages),
                        (GDestroyNotify)chatbot_message_array_free);
  g_task_run_in_thread (task, _act_thread);
  g_object_unref (task);
}

/**
 * chatbot_agent_act_finish:
 * @agent: Agent.
 * @result: A %GAsyncResult.
 * @error: (out) (optional): Location to store error.
 *
 * Finishes operation started with [method@Chatbot.Agent.act_async].
 *
 * Returns: The messages agent has generated.
 */
ChatbotMessageArray *
chatbot_agent_act_finish (ChatbotAgent *agent, GAsyncResult *result,
                          GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_AGENT (agent), NULL);
  g_return_val_if_fail (
      G_IS_ASYNC_RESULT (result)
          && (G_OBJECT (agent) == g_async_result_get_source_object (result)),
      NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

/**
 * chatbot_agent_add_tool:
 * @agent: Agent
 * @tool: A [iface@Chatbot.Tool].
 * @error: (out) (optional): Location to store error.
 *
 * Adds a tool.
 *
 * Adding a tool fails if:
 *
 * 1. @tool is already added.
 *
 * 2. Module name conflict.
 *
 * Returns: %TRUE if @tool is added, or %FALSE if @tool is not added.
 */
gboolean
chatbot_agent_add_tool (ChatbotAgent *agent, ChatbotTool *tool, GError **error)
{
  ChatbotAgentInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_AGENT (agent), FALSE);
  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);

  iface = CHATBOT_AGENT_GET_IFACE (agent);
  g_return_val_if_fail (iface->add_tool != NULL, FALSE);
  return iface->add_tool (agent, tool, error);
}

/**
 * chatbot_agent_remove_tool:
 * @agent: Agent
 * @tool: A [iface@Chatbot.Tool].
 *
 * Removes a tool.
 *
 * Returns: %TRUE if @tool is removed, or %FALSE if @tool is not removed.
 */
gboolean
chatbot_agent_remove_tool (ChatbotAgent *agent, ChatbotTool *tool)
{
  ChatbotAgentInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_AGENT (agent), FALSE);
  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), FALSE);

  iface = CHATBOT_AGENT_GET_IFACE (agent);
  g_return_val_if_fail (iface->remove_tool != NULL, FALSE);
  return iface->remove_tool (agent, tool);
}
