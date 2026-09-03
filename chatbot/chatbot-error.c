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
 * ChatbotError:
 *
 * Errors used in Chatbot.
 */

#include <chatbot/chatbot-error.h>

G_DEFINE_QUARK (chatbot-error, chatbot_error);

G_DEFINE_ENUM_TYPE (ChatbotError, chatbot_error,
                    /**
                     * CHATBOT_ERROR_UNKNOWN:
                     *
                     * Unknown or undefined error. Debug or Experimental only.
                     */
                    G_DEFINE_ENUM_VALUE (CHATBOT_ERROR_UNKNOWN, "unknown"),
                    /**
                     * CHATBOT_ERROR_NOT_FOUND:
                     *
                     * General "not found" error.
                     */
                    G_DEFINE_ENUM_VALUE (CHATBOT_ERROR_NOT_FOUND, "not-found"),
                    /**
                     * CHATBOT_ERROR_MODULE:
                     *
                     * General module related error.
                     */
                    G_DEFINE_ENUM_VALUE (CHATBOT_ERROR_MODULE, "module"),
                    /**
                     * CHATBOT_ERROR_INVALID:
                     *
                     * General invalid data error.
                     */
                    G_DEFINE_ENUM_VALUE (CHATBOT_ERROR_INVALID, "invalid"));
