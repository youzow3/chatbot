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

#include <test/chatbot/chatbot-test-agent.h>

struct _ChatbotTestAgent
{
  ChatbotModule parent;
  GPtrArray *tools;
};

static void chatbot_test_agent_agent_iface_init (ChatbotAgentInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (
    ChatbotTestAgent, chatbot_test_agent, CHATBOT_TYPE_MODULE,
    G_IMPLEMENT_INTERFACE (CHATBOT_TYPE_AGENT,
                           chatbot_test_agent_agent_iface_init));

static ChatbotMessageArray *
chatbot_test_agent_act (ChatbotAgent *agent, ChatbotMessageArray *messages,
                        GCancellable *cancellable, GError **error)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_AGENT (agent), NULL);
  g_return_val_if_fail (messages != NULL, NULL);
  g_return_val_if_fail (
      (cancellable == NULL) || G_IS_CANCELLABLE (cancellable), NULL);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), NULL);

  return chatbot_message_array_copy (messages);
}

static gboolean
chatbot_test_agent_add_tool (ChatbotAgent *agent, ChatbotTool *tool,
                             GError **error)
{
	ChatbotTestAgent *test_agent;

  g_return_val_if_fail (CHATBOT_IS_TEST_AGENT (agent), FALSE);
  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), FALSE);
  g_return_val_if_fail ((error == NULL) || (*error == NULL), FALSE);
  test_agent = CHATBOT_TEST_AGENT(agent);

  g_object_ref(tool);
  g_ptr_array_add(test_agent->tools, tool);

  return TRUE;
}

static gboolean
chatbot_test_agent_remove_tool (ChatbotAgent *agent, ChatbotTool *tool)
{
	ChatbotTestAgent *test_agent;
  g_return_val_if_fail (CHATBOT_IS_TEST_AGENT (agent), FALSE);
  g_return_val_if_fail (CHATBOT_IS_TOOL (tool), FALSE);
  test_agent = CHATBOT_TEST_AGENT(agent);

  g_ptr_array_remove(test_agent->tools, tool);

  return TRUE;
}

static void
chatbot_test_agent_agent_iface_init (ChatbotAgentInterface *iface)
{
  iface->act = chatbot_test_agent_act;
  iface->add_tool = chatbot_test_agent_add_tool;
  iface->remove_tool = chatbot_test_agent_remove_tool;
}

static const gchar *
chatbot_test_agent_get_name (ChatbotModule *module)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_AGENT (module), NULL);
  return "chatbot-test-agent";
}

static const gchar *
chatbot_test_agent_get_description (ChatbotModule *module)
{
  g_return_val_if_fail (CHATBOT_IS_TEST_AGENT (module), NULL);
  return "chatbot test agent";
}

static void chatbot_test_agent_dispose(GObject *object)
{
	ChatbotTestAgent *agent;

	g_return_if_fail(CHATBOT_IS_TEST_AGENT(object));

	agent = CHATBOT_TEST_AGENT(object);
	g_clear_pointer(&agent->tools, g_ptr_array_unref);

	G_OBJECT_CLASS(chatbot_test_agent_parent_class)->dispose(object);
}

static void
chatbot_test_agent_class_init (ChatbotTestAgentClass *klass)
{
  ChatbotModuleClass *module_class = CHATBOT_MODULE_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

  module_class->get_name = chatbot_test_agent_get_name;
  module_class->get_description = chatbot_test_agent_get_description;
  object_class->dispose = chatbot_test_agent_dispose;
}

static void
chatbot_test_agent_init (ChatbotTestAgent *agent)
{
	agent->tools = g_ptr_array_new_with_free_func(g_object_unref);
}

ChatbotTestAgent *
chatbot_test_agent_new (void)
{
  return g_object_new (CHATBOT_TYPE_TEST_AGENT, NULL);
}
