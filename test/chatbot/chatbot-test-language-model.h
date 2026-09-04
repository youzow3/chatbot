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

#define CHATBOT_TYPE_TEST_LANGUAGE_MODEL                                      \
  chatbot_test_language_model_get_type ()
G_DECLARE_FINAL_TYPE (ChatbotTestLanguageModel, chatbot_test_language_model,
                      CHATBOT, TEST_LANGUAGE_MODEL, ChatbotModule);

ChatbotTestLanguageModel *chatbot_test_language_model_new (void);

G_END_DECLS
