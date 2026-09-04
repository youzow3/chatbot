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

#include <test/chatbot/chatbot-test-error.h>

G_DEFINE_QUARK(chatbot-test-error, chatbot_test_error);

G_DEFINE_ENUM_TYPE (ChatbotTestError, chatbot_test_error,
                    G_DEFINE_ENUM_VALUE (CHATBOT_TEST_ERROR_EXPECTED,
                                         "expected"),
                    G_DEFINE_ENUM_VALUE (CHATBOT_TEST_ERROR_SWITCHED,
                                         "switched"));
