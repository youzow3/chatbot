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
 * ChatbotTool:
 *
 * Interface to define tool used by language models.
 */

#include <chatbot/chatbot-tool.h>

/**
 * chatbot_tool_arg_ref:
 * @arg: self
 *
 * Increments the ref count.
 *
 * Increments the ref count or no-op if @arg is statically allocated.
 */
void
chatbot_tool_arg_ref (ChatbotToolArg *arg)
{
  g_return_if_fail (arg != NULL);

  if (arg->ref == -1)
    return;
  arg->ref++;
}

/**
 * chatbot_tool_arg_unref:
 * @arg: self
 *
 * Decrements the ref count.
 *
 * Decrements the ref count and frees up @arg, `name`, `signature`, and
 * `description` in @arg when ref count reached to 0.
 *
 * No-op if @arg is statically allocated.
 */
void
chatbot_tool_arg_unref (ChatbotToolArg *arg)
{
  g_return_if_fail (arg != NULL);

  if (arg->ref == -1)
    return;
  if (--arg->ref != 0)
    return;
  g_free (arg->name);
  g_free (arg->signature);
  g_free (arg->description);
  g_free (arg);
}

/**
 * chatbot_tool_function_ref:
 * @func: self
 *
 * Increments the ref count.
 *
 * Increments the ref count or no-op if @func is statically allocated.
 */
void
chatbot_tool_function_ref (ChatbotToolFunction *func)
{
  g_return_if_fail (func != NULL);
  if (func->ref == -1)
    return;
  func->ref++;
}

/**
 * chatbot_tool_function_unref:
 * @func: self
 *
 * Decrements the ref count.
 *
 * Decrements the ref count and frees up @func, `name`, `title`, `description`
 * and `inputs`, and `outputs` in @func when ref count reached to 0.
 *
 * No-op if @func is statically allocated.
 */
void
chatbot_tool_function_unref (ChatbotToolFunction *func)
{
  g_return_if_fail (func != NULL);
  if (func->ref == -1)
    return;
  if (--func->ref != 0)
    return;
  g_free (func->name);
  g_free (func->title);
  g_free (func->description);
  for (gsize i = 0; (func->inputs != NULL) && (func->inputs[i] != NULL); i++)
    chatbot_tool_arg_unref (func->inputs[i]);
  g_free (func->inputs);
  for (gsize i = 0; (func->outputs != NULL) && (func->outputs[i] != NULL); i++)
    chatbot_tool_arg_unref (func->outputs[i]);
  g_free (func->outputs);
  g_free (func);
}

enum
{
  LIST_CHANGED,
  N_SIGNALS
};

guint signals[N_SIGNALS];

G_DEFINE_INTERFACE (ChatbotTool, chatbot_tool, CHATBOT_TYPE_MODULE);

static void
chatbot_tool_default_init (ChatbotToolInterface *iface)
{
  /**
   * ChatbotTool::list-changed:
   * @tool: self
   *
   * Emitted when available functions are changed.
   *
   * This signal should be compatible with MCP's `listChanged` notification.
   */
  signals[LIST_CHANGED]
      = g_signal_new ("list-changed", CHATBOT_TYPE_TOOL, G_SIGNAL_RUN_FIRST, 0,
                      NULL, NULL, NULL, G_TYPE_NONE, 0);
}

/**
 * chatbot_tool_get_functions:
 * @tool: self
 *
 * Gets available functions.
 *
 * Returns: (transfer none) (array zero-terminated=1): Available
 * functions array.
 */
ChatbotToolFunction **
chatbot_tool_get_functions (ChatbotTool *tool)
{
  ChatbotToolInterface *iface;
  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), NULL);
  iface = CHATBOT_TOOL_GET_IFACE (tool);
  g_return_val_if_fail (iface->get_functions != NULL, NULL);
  return iface->get_functions (tool);
}

/**
 * chatbot_tool_call:
 * @tool: self
 * @name: function name to call.
 * @args: arguments for function.
 * @cancellable: #GCancellable to cancel operation.
 * @error: (out) (nullable) (optional): Location to store the error.
 *
 * Calls a function in the tool.
 *
 * Type signature of @args and returned value must be `a{sv}`.
 *
 * Returns: (transfer full): A #GVariant containing result.
 */
GVariant *
chatbot_tool_call (ChatbotTool *tool, const gchar *name, GVariant *args,
                   GCancellable *cancellable, GError **error)
{
  ChatbotToolInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail ((args != NULL)
                            && g_variant_type_equal (g_variant_get_type (args),
                                                     G_VARIANT_TYPE_VARDICT),
                        NULL);
  g_return_val_if_fail (
      (cancellable == NULL) || G_IS_CANCELLABLE (cancellable), NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);
  iface = CHATBOT_TOOL_GET_IFACE (tool);
  g_return_val_if_fail (iface->call != NULL, NULL);
  return iface->call (tool, name, args, cancellable, error);
}

typedef struct
{
  gchar *name;
  GVariant *args;
} _CALL_THREAD_DATA;

_CALL_THREAD_DATA *
_call_thread_data_new (const gchar *name, GVariant *args)
{
  _CALL_THREAD_DATA *th_data;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (args != NULL, NULL);

  th_data = g_new (_CALL_THREAD_DATA, 1);
  th_data->name = g_strdup (name);
  g_variant_ref_sink (args);
  th_data->args = args;
  return th_data;
}

void
_call_thread_data_free (_CALL_THREAD_DATA *th_data)
{
  g_free (th_data->name);
  g_variant_unref (th_data->args);
  g_free (th_data);
}

static void
_call_thread (GTask *task, gpointer source_object, gpointer task_data,
              GCancellable *cancellable)
{
  _CALL_THREAD_DATA *th_data;
  GVariant *ret;
  GError *error = NULL;

  g_return_if_fail (G_IS_TASK (task));
  g_return_if_fail (CHATBOT_IS_TOOL (source_object));
  g_return_if_fail (task_data != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));
  th_data = task_data;

  ret = chatbot_tool_call (CHATBOT_TOOL (source_object), th_data->name,
                           th_data->args, cancellable, &error);
  if (ret == NULL)
    {
      g_task_return_error (task, error);
      return;
    }
  g_task_return_pointer (task, ret, NULL);
}

/**
 * chatbot_tool_call_async:
 * @tool: self
 * @name: function name to call.
 * @args: arguments for function.
 * @cancellable: #GCancellable to cancel operation.
 * @callback: (nullable): A #GAsyncReadyCallback.
 * @user_data: (nullable): Data passed to @callback.
 *
 * Asynchronous version of [method@Chatbot.Tool.call].
 */
void
chatbot_tool_call_async (ChatbotTool *tool, const gchar *name, GVariant *args,
                         GCancellable *cancellable,
                         GAsyncReadyCallback callback, gpointer user_data)
{
  GTask *task;

  g_return_if_fail (CHATBOT_IS_TOOL (tool));
  g_return_if_fail (name != NULL);
  g_return_if_fail (args != NULL);
  g_return_if_fail ((cancellable == NULL) || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (tool, cancellable, callback, user_data);
  g_task_set_task_data (task, _call_thread_data_new (name, args),
                        (GDestroyNotify)_call_thread_data_free);
  g_task_run_in_thread (task, _call_thread);
  g_object_unref (task);
}

/**
 * chatbot_tool_call_finish:
 * @tool: self
 * @result: A #GAsyncResult.
 * @error: (out) (optional): Location to store runtime error.
 *
 * Finishes tool calling started with [method@Chatbot.Tool.call_async].
 *
 * Returns: (transfer full): A #GVariant containing result.
 */
GVariant *
chatbot_tool_call_finish (ChatbotTool *tool, GAsyncResult *result,
                          GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), NULL);
  g_return_val_if_fail (
      G_IS_ASYNC_RESULT (result)
          && (G_OBJECT (tool) == g_async_result_get_source_object (result)),
      NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}
