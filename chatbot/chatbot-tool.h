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

#include "chatbot-language-model.h"
#include "chatbot-module.h"

G_BEGIN_DECLS

#define CHATBOT_TYPE_TOOL chatbot_tool_get_type ()
G_DECLARE_INTERFACE (ChatbotTool, chatbot_tool, CHATBOT, TOOL, ChatbotModule);

#define CHATBOT_TYPE_TOOL_ARG chatbot_tool_arg_get_type ()
GType chatbot_tool_arg_get_type (void) G_GNUC_CONST;

/**
 * ChatbotToolArg:
 * @name: arg name
 * @description: arg description
 * @type: GVariant type
 * @required: TRUE if the arg is required
 * @ref: -1 if the structure is statically defined
 *
 * Valid basic type of @type is "b", "x", "d", "s", and "a".
 */
typedef struct _ChatbotToolArg
{
  gchar *name;
  gchar *description;
  gchar *type;
  guint ref;
} ChatbotToolArg;

ChatbotToolArg *chatbot_tool_arg_new (const gchar *name,
                                      const gchar *description,
                                      const gchar *type);
ChatbotToolArg *chatbot_tool_arg_ref (ChatbotToolArg *arg);
void chatbot_tool_arg_unref (ChatbotToolArg *arg);

#define CHATBOT_TYPE_TOOL_FUNCTION chatbot_tool_function_get_type ()
GType chatbot_tool_function_get_type (void) G_GNUC_CONST;

/**
 * ChatbotToolFunction:
 * @name: function name
 * @description: function description
 * @input_schemas: (array zero-terminated=1): input(s) information
 * @output_schemas: (array zero-terminated=1): output(s) information
 * @ref: -1 if the structure is statically defined
 */
typedef struct _ChatbotToolFunction
{
  gchar *name;
  gchar *description;
  ChatbotToolArg **input_schemas;
  ChatbotToolArg **output_schemas;
  guint ref;
} ChatbotToolFunction;

ChatbotToolFunction *chatbot_tool_function_new (
    const gchar *name, const gchar *description,
    ChatbotToolArg **input_schemas, gsize input_schemas_len,
    ChatbotToolArg **output_schemas, gsize output_schemas_len);
ChatbotToolFunction *chatbot_tool_function_ref (ChatbotToolFunction *function);
void chatbot_tool_function_unref (ChatbotToolFunction *function);

struct _ChatbotToolInterface
{
  GTypeInterface iface;
  const ChatbotToolFunction *const *(*get_function_definitions) (
      ChatbotTool *tool);
  GVariantDict *(*call_function) (ChatbotTool *tool,
                                  const gchar *function_name,
                                  GVariantDict *parameters,
                                  ChatbotLanguageModel *language_model,
                                  GCancellable *cancellable, GError **error);
};

gpointer chatbot_tool_new (GType type);
const ChatbotToolFunction *const *
chatbot_tool_get_function_definitions (ChatbotTool *tool);
GVariantDict *chatbot_tool_call_function (ChatbotTool *tool,
                                          const gchar *function_name,
                                          GVariantDict *parameters,
                                          ChatbotLanguageModel *language_model,
                                          GCancellable *cancellable,
                                          GError **error);

G_END_DECLS
