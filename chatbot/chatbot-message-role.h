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

#include <glib-object.h>

G_BEGIN_DECLS

#define CHATBOT_TYPE_MESSAGE_ROLE chatbot_message_role_get_type ()
GType chatbot_message_role_get_type (void) G_GNUC_CONST;

typedef enum _ChatbotMessageRole
{
  /**
   * CHATBOT_MESSAGE_ROLE_NONE:
   *
   * No role, or invalid role
   */
  CHATBOT_MESSAGE_ROLE_NONE,
  /**
   * CHATBOT_MESSAGE_ROLE_SYSTEM:
   *
   * System role
   */
  CHATBOT_MESSAGE_ROLE_SYSTEM,
  /**
   * CHATBOT_MESSAGE_ROLE_USER:
   *
   * User role
   */
  CHATBOT_MESSAGE_ROLE_USER,
  /**
   * CHATBOT_MESSAGE_ROLE_TOOL_RESPONSE:
   *
   * Tool role (tool calling responses)
   */
  CHATBOT_MESSAGE_ROLE_TOOL_RESPONSE,
  /**
   * CHATBOT_MESSAGE_ROLE_ASSISTANT:
   *
   * Assistant role
   */
  CHATBOT_MESSAGE_ROLE_ASSISTANT,
  /**
   * CHATBOT_MESSAGE_ROLE_ASSISTANT_THINK:
   *
   * Assistant role (think)
   */
  CHATBOT_MESSAGE_ROLE_ASSISTANT_THINK,
  /**
   * CHATBOT_MESSAGE_ROLE_ASSISTANT_THINK_INTERNAL:
   *
   * Assistant role (internal think)
   */
  CHATBOT_MESSAGE_ROLE_ASSISTANT_THINK_INTERNAL,
  /**
   * CHATBOT_MESSAGE_ROLE_ASSISTANT_CALL:
   *
   * Assistant role (tool/function call)
   */
  CHATBOT_MESSAGE_ROLE_ASSISTANT_CALL,
} ChatbotMessageRole;

G_END_DECLS
