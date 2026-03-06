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
 * ChatbotChatData:
 *
 * [iface@ChatbotData] implementation for dialogue data. This class implements
 * [method@ChatbotData.get_strings]. This class is not immutable style, and
 * thus, data retrieved with [method@ChatbotData.get_strings] is not guaranteed
 * to be valid until instance is freed.
 *
 * Value returned with [method@ChatbotData.get_strings] will be changed and
 * invalidated when [method@ChatbotChatData.append] is called.
 */

#include "chatbot-data.h"

#include "chatbot-chat-data.h"

typedef struct
{
  GStrvBuilder *builder;
  GStrv role_and_messages;
} ChatbotChatDataPrivate;

static void chatbot_chat_data_data_init (ChatbotDataInterface *iface);

G_DEFINE_TYPE_EXTENDED (ChatbotChatData, chatbot_chat_data, G_TYPE_OBJECT, 0,
                        G_ADD_PRIVATE (ChatbotChatData) G_IMPLEMENT_INTERFACE (
                            CHATBOT_TYPE_DATA, chatbot_chat_data_data_init));

static const GStrv
chatbot_chat_data_get_strings (ChatbotData *data)
{
  ChatbotChatDataPrivate *priv;

  g_return_val_if_fail (CHATBOT_IS_CHAT_DATA (data), NULL);

  priv = chatbot_chat_data_get_instance_private (CHATBOT_CHAT_DATA (data));
  if (priv->builder == NULL)
    return priv->role_and_messages;
  g_clear_pointer (&priv->role_and_messages, g_strfreev);
  priv->role_and_messages = g_strv_builder_end (priv->builder);
  g_clear_pointer (&priv->builder, g_strv_builder_unref);
  return priv->role_and_messages;
}

static void
chatbot_chat_data_data_init (ChatbotDataInterface *iface)
{
  iface->get_strings = chatbot_chat_data_get_strings;
}

static void
chatbot_chat_data_dispose (GObject *object)
{
  ChatbotChatDataPrivate *priv
      = chatbot_chat_data_get_instance_private (CHATBOT_CHAT_DATA (object));
  g_clear_pointer (&priv->builder, g_strv_builder_unref);
  G_OBJECT_CLASS (chatbot_chat_data_parent_class)->dispose (object);
}

static void
chatbot_chat_data_finalize (GObject *object)
{
  ChatbotChatDataPrivate *priv
      = chatbot_chat_data_get_instance_private (CHATBOT_CHAT_DATA (object));
  g_strfreev (priv->role_and_messages);
  G_OBJECT_CLASS (chatbot_chat_data_parent_class)->finalize (object);
}

static void
chatbot_chat_data_class_init (ChatbotChatDataClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = chatbot_chat_data_dispose;
  object_class->finalize = chatbot_chat_data_finalize;
}

static void
chatbot_chat_data_init (ChatbotChatData *init)
{
}

ChatbotChatData *
chatbot_chat_data_new (void)
{
  return CHATBOT_CHAT_DATA (g_object_new (CHATBOT_TYPE_CHAT_DATA, NULL));
}

/**
 * chatbot_chat_data_append:
 * @role: role
 * @message: message
 *
 * Append message to data
 *
 * Any kind and format are supported for @role and @message and this function
 * doesn't do anything to normalize them.
 */
void
chatbot_chat_data_append (ChatbotChatData *chat_data, const gchar *role,
                          const gchar *message)
{
  ChatbotChatDataPrivate *priv;

  g_return_if_fail (CHATBOT_IS_CHAT_DATA (chat_data));
  g_return_if_fail (role != NULL);
  g_return_if_fail (message != NULL);

  priv = chatbot_chat_data_get_instance_private (chat_data);
  if (priv->builder == NULL)
    priv->builder = g_strv_builder_new ();
  g_strv_builder_add_many (priv->builder, role, message, NULL);
}
