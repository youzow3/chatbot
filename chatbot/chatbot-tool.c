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
 * Returns: (transfer full): A %GVariant containing result.
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
