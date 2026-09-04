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

#include <chatbot/chatbot-module.h>
#include <gio/gio.h>

G_BEGIN_DECLS

/**
 * ChatbotToolArg:
 * @ref: The reference count or -1 if statically allocated.
 * @name: Name of the argument.
 * @signature: #GVariant type signature.
 * @description: Description of the argument.
 * @user: %TRUE if the argument is provided by/for user, not LM.
 *
 * Represents argument for function input or output.
 *
 * @signature can be any definite type include maybe type.
 */
typedef struct _ChatbotToolArg
{
  gint ref;
  gchar *name;
  gchar *signature;
  gchar *description;
  gboolean user;
} ChatbotToolArg;

void chatbot_tool_arg_ref (ChatbotToolArg *arg);
void chatbot_tool_arg_unref (ChatbotToolArg *arg);

/**
 * ChatbotToolFunction:
 * @ref: The reference count or -1 if statically allocated.
 * @name: Name of the function.
 * @title: (nullable): Human readable function name for display purposes.
 * @description: Description of the argument.
 * @inputs: (nullable) (array zero-terminated=1): input arguments or %NULL for
 * no input arguments.
 * @outputs: (nullable) (array zero-terminated=1): output arguments or %NULL
 * for no output arguments.
 *
 * Represents function for tool calling.
 */
typedef struct _ChatbotToolFunction
{
  gint ref;
  gchar *name;
  gchar *title;
  gchar *description;
  ChatbotToolArg **inputs;
  ChatbotToolArg **outputs;
} ChatbotToolFunction;

void chatbot_tool_function_ref (ChatbotToolFunction *func);
void chatbot_tool_function_unref (ChatbotToolFunction *func);

#define CHATBOT_TYPE_TOOL chatbot_tool_get_type ()
G_DECLARE_INTERFACE (ChatbotTool, chatbot_tool, CHATBOT, TOOL, ChatbotModule);

struct _ChatbotToolInterface
{
  GTypeInterface iface;
  ChatbotToolFunction **(*get_functions) (ChatbotTool *tool);
  GVariant *(*call) (ChatbotTool *tool, const gchar *name, GVariant *args,
                     GCancellable *cancellable, GError **error);
};

ChatbotToolFunction **chatbot_tool_get_functions (ChatbotTool *tool);
GVariant *chatbot_tool_call (ChatbotTool *tool, const gchar *name,
                             GVariant *args, GCancellable *cancellable,
                             GError **error);
void chatbot_tool_call_async (ChatbotTool *tool, const gchar *name,
                              GVariant *args, GCancellable *cancellable,
                              GAsyncReadyCallback callback,
                              gpointer user_data);
GVariant *chatbot_tool_call_finish (ChatbotTool *tool, GAsyncResult *result,
                                    GError **error);

G_END_DECLS
