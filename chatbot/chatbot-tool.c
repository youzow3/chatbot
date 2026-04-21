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
 * External tool interface for Chatbot.
 *
 * Implementing all methods is required.
 */

#include "chatbot-tool.h"

G_DEFINE_BOXED_TYPE (ChatbotToolArg, chatbot_tool_arg, chatbot_tool_arg_ref,
                     chatbot_tool_arg_unref);

/**
 * chatbot_tool_arg_new:
 * @name: arg name
 * @description: arg description
 * @type: GVariant compatible type signature
 * @required: TRUE if required arg, FALSE for not.
 *
 * Helper function to dynamically allocate [struct@ChatbotToolArg].
 *
 * Returns: Newly created floating [struct@ChatbotToolArg]
 */
ChatbotToolArg *
chatbot_tool_arg_new (const gchar *name, const gchar *description,
                      const gchar *type, gboolean required)
{
  ChatbotToolArg *arg;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (description != NULL, NULL);
  g_return_val_if_fail (g_variant_is_signature (type), NULL);

  arg = g_new (ChatbotToolArg, 1);
  arg->name = g_strdup (name);
  arg->description = g_strdup (description);
  arg->type = g_strdup (type);
  arg->required = required;
  arg->ref = 0;
  return arg;
}

/**
 * chatbot_tool_arg_ref:
 * @arg: arg
 *
 * Returns: @arg
 */
ChatbotToolArg *
chatbot_tool_arg_ref (ChatbotToolArg *arg)
{
  g_return_val_if_fail (arg != NULL, NULL);
  if (arg->ref != -1)
    arg->ref++;
  return arg;
}

void
chatbot_tool_arg_unref (ChatbotToolArg *arg)
{
  g_return_if_fail (arg != NULL);
  if (arg->ref == -1)
    return;
  if (--arg->ref == 0)
    {
      g_free (arg->name);
      g_free (arg->description);
      g_free (arg->type);
      g_free (arg);
    }
}

G_DEFINE_BOXED_TYPE (ChatbotToolFunction, chatbot_tool_function,
                     chatbot_tool_function_ref, chatbot_tool_function_unref);

/**
 * chatbot_tool_function_new:
 * @name: function name
 * @description: function description
 * @input_schemas: (nullable) (array length=input_schemas_len): function input
 * schemas
 * @input_schemas_len: length of @input_schemas
 * @output_schemas: (nullable) (array length=output_schemas_len): function
 * output schemas
 * @output_schemas_len: length of @output_schemas
 *
 * Helper function to dynamically define [struct@ChatbotToolFunction].
 *
 * Returns: newly created floating [struct@ChatbotToolFunction]
 */
ChatbotToolFunction *
chatbot_tool_function_new (const gchar *name, const gchar *description,
                           ChatbotToolArg **input_schemas,
                           gsize input_schemas_len,
                           ChatbotToolArg **output_schemas,
                           gsize output_schemas_len)
{
  ChatbotToolFunction *function;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (description != NULL, NULL);
  g_return_val_if_fail ((input_schemas_len == 0) || (input_schemas != NULL),
                        NULL);
  g_return_val_if_fail ((output_schemas_len == 0) || (output_schemas != NULL),
                        NULL);

  function = g_new (ChatbotToolFunction, 1);
  function->name = g_strdup (name);
  function->description = g_strdup (description);
  function->input_schemas = g_new (ChatbotToolArg *, input_schemas_len + 1);
  for (gsize i = 0; i < input_schemas_len; i++)
    {
      chatbot_tool_arg_ref (input_schemas[i]);
      function->input_schemas[i] = input_schemas[i];
    }
  function->input_schemas[input_schemas_len] = NULL;
  function->output_schemas = g_new (ChatbotToolArg *, output_schemas_len + 1);
  for (gsize i = 0; i < output_schemas_len; i++)
    {
      chatbot_tool_arg_ref (output_schemas[i]);
      function->output_schemas[i] = output_schemas[i];
    }
  function->output_schemas[output_schemas_len] = NULL;
  function->ref = 0;
  return function;
}

/**
 * chatbot_tool_function_ref:
 * @function: function
 *
 * Returns: @function
 */
ChatbotToolFunction *
chatbot_tool_function_ref (ChatbotToolFunction *function)
{
  g_return_val_if_fail (function != NULL, NULL);
  if (function->ref != -1)
    function->ref++;
  return function;
}

/**
 * chatbot_tool_function_unref:
 * @function: function
 */
void
chatbot_tool_function_unref (ChatbotToolFunction *function)
{
  g_return_if_fail (function != NULL);
  if (function->ref == -1)
    return;
  if (--function->ref == 0)
    {
      g_free (function->name);
      g_free (function->description);
      if (function->input_schemas)
        for (ChatbotToolArg **i = function->input_schemas; *i; i++)
          chatbot_tool_arg_unref (*i);
      if (function->output_schemas)
        for (ChatbotToolArg **i = function->output_schemas; *i; i++)
          chatbot_tool_arg_unref (*i);
      g_free (function->output_schemas);
      g_free (function->input_schemas);
      g_free (function);
    }
}

enum
{
  FUNCTIONS_CHANGED,
  N_SIGNALS
};

int signals[N_SIGNALS];

G_DEFINE_INTERFACE (ChatbotTool, chatbot_tool, CHATBOT_TYPE_MODULE);

static void
chatbot_tool_default_init (ChatbotToolInterface *iface)
{
  /**
   * ChatbotTool:name:
   *
   * Tool name
   */
  g_object_interface_install_property (
      iface, g_param_spec_string ("name", "name", "tool name", NULL,
                                  G_PARAM_READABLE));

  /**
   * ChatbotTool:description
   *
   * Tool description
   */
  g_object_interface_install_property (
      iface, g_param_spec_string ("description", "description",
                                  "tool description", NULL, G_PARAM_READABLE));

  /**
   * ChatbotTool:functions
   *
   * Function definitions tool provides
   */
  g_object_interface_install_property (
      iface,
      g_param_spec_boxed ("functions", "functions", "function definitions",
                          G_TYPE_PTR_ARRAY, G_PARAM_READABLE));

  /**
   * ChatbotTool::functions-changed:
   *
   * Signal that emitted when [property@ChatbotTool:functions] changed.
   */
  signals[FUNCTIONS_CHANGED]
      = g_signal_new ("functions-changed", CHATBOT_TYPE_TOOL,
                      G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

/**
 * chatbot_tool_new:
 *
 * Currently just wrapper of g_object_new without properties.
 */
gpointer
chatbot_tool_new (GType type)
{
  return g_object_new (type, NULL);
}

/**
 * chatbot_tool_get_function_definitions: (get-property functions):
 *
 * Gets [property@ChatbotTool:functions] of the instance. This should be more
 * efficient than calling [method@G.Object.get_property] because this doesn't
 * need to get value from #GArray and #GValue.
 *
 * Returns: (array zero-terminated=1) (transfer none): Function definitions
 * array instance own
 */
const ChatbotToolFunction *const *
chatbot_tool_get_function_definitions (ChatbotTool *tool)
{
  ChatbotToolInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), NULL);
  iface = CHATBOT_TOOL_GET_IFACE (tool);
  g_return_val_if_fail (iface->get_function_definitions != NULL, NULL);
  return iface->get_function_definitions (tool);
}

/**
 * chatbot_tool_call_function:
 * @function_name: function name to call
 * @parameters: (nullable): call parameters
 * @language_model: (nullable): language model instance
 * @cancellable: (nullable): cancellable to cancel operation
 * @error: (nullable): pointer to the #GError* to store error
 *
 * Call function
 *
 * The valid function and its parameters should be function definitions defined
 * at [property@ChatbotTool:functions]
 *
 * @language_model can be used to make tool calling interactive. Though it
 * depends on the user of tools, so, if there is any interactive only tool, the
 * implementer should check valid @language_model is supplied before using it.
 *
 * Returns:Tuple of return values
 */
GVariant *
chatbot_tool_call_function (ChatbotTool *tool, const gchar *function_name,
                            GVariant *parameters,
                            ChatbotLanguageModel *language_model,
                            GCancellable *cancellable, GError **error)
{
  ChatbotToolInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), NULL);
  g_return_val_if_fail (function_name != NULL, NULL);
  g_return_val_if_fail (CHATBOT_IS_LANGUAGE_MODEL (language_model)
                            || (language_model == NULL),
                        NULL);
  g_return_val_if_fail (
      G_IS_CANCELLABLE (cancellable) || (cancellable == NULL), NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);
  iface = CHATBOT_TOOL_GET_IFACE (tool);
  g_return_val_if_fail (iface->call_function != NULL, NULL);
  return iface->call_function (tool, function_name, parameters, language_model,
                               cancellable, error);
}
