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
 * ChatbotMessageRole:
 *
 * Enumeration for message role.
 *
 * Authority level should be following:
 *
 * 1. %CHATBOT_MESSAGE_ROLE_SYSTEM *(Highest authority level)*
 *
 * 2. %CHATBOT_MESSAGE_ROLE_USER
 *
 * 3. %CHATBOT_MESSAGE_ROLE_TOOL_RESPONSE
 *
 * The difference between %CHATBOT_MESSAGE_ROLE_ASSISTANT_THINK and
 * %CHATBOT_MESSAGE_ROLE_ASSISTANT_THINK_INTERNAL are whether the thinking
 * traces are safe to be shown to end users or not. For example, in OpenAI
 * Harmony format, `final` is safe to be shown, and `analysis` is not.
 */

#include <chatbot/chatbot-message-role.h>

G_DEFINE_ENUM_TYPE (
    ChatbotMessageRole, chatbot_message_role,
    G_DEFINE_ENUM_VALUE (CHATBOT_MESSAGE_ROLE_NONE, "none"),
    G_DEFINE_ENUM_VALUE (CHATBOT_MESSAGE_ROLE_SYSTEM, "system"),
    G_DEFINE_ENUM_VALUE (CHATBOT_MESSAGE_ROLE_USER, "user"),
    G_DEFINE_ENUM_VALUE (CHATBOT_MESSAGE_ROLE_TOOL_RESPONSE, "tool-response"),
    G_DEFINE_ENUM_VALUE (CHATBOT_MESSAGE_ROLE_ASSISTANT, "assistant"),
    G_DEFINE_ENUM_VALUE (CHATBOT_MESSAGE_ROLE_ASSISTANT_THINK,
                         "assistant-think"),
    G_DEFINE_ENUM_VALUE (CHATBOT_MESSAGE_ROLE_ASSISTANT_THINK_INTERNAL,
                         "assistant-think-internal"),
    G_DEFINE_ENUM_VALUE (CHATBOT_MESSAGE_ROLE_ASSISTANT_CALL,
                         "assistant-call"));
