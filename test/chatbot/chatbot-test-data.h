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

#include <chatbot/chatbot.h>

G_BEGIN_DECLS

#define CHATBOT_TEST_DATA_DATA "This is dummy data"
#define CHATBOT_TEST_DATA_MIME_TYPE "dummy/dummy"

#define CHATBOT_TYPE_TEST_DATA chatbot_test_data_get_type ()
G_DECLARE_FINAL_TYPE (ChatbotTestData, chatbot_test_data, CHATBOT, TEST_DATA,
                      GObject);

ChatbotTestData *chatbot_test_data_new (void);

G_END_DECLS
