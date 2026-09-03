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
 * ChatbotMultiAgent:
 *
 * Interface to define multi agent system (MAS).
 */

#include <chatbot/chatbot-multi-agent.h>

G_DEFINE_INTERFACE (ChatbotMultiAgent, chatbot_multi_agent,
                    CHATBOT_TYPE_AGENT);

static void
chatbot_multi_agent_default_init (ChatbotMultiAgentInterface *iface)
{
}

/**
 * chatbot_multi_agent_get_agents:
 * @multi_agent: self
 * @len: (out) (optional): Location to store number of agents.
 *
 * Gets agents used in multi-agent system.
 *
 * Returns: (array) (transfer none): Array of [iface@Chatbot.Agent] used in
 * @multi_agent.
 */
ChatbotAgent **
chatbot_multi_agent_get_agents (ChatbotMultiAgent *multi_agent, gsize *len)
{
  ChatbotMultiAgentInterface *iface;

  g_return_val_if_fail (CHATBOT_IS_MULTI_AGENT (multi_agent), NULL);
  iface = CHATBOT_MULTI_AGENT_GET_IFACE (multi_agent);
  g_return_val_if_fail (iface->get_agents != NULL, NULL);
  return iface->get_agents (multi_agent, len);
}
