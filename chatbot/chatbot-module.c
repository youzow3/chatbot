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
 * ChatbotModule:
 *
 * Base class for any modules.
 */

#include "chatbot-module.h"

#include <gio/gio.h>
#include <gmodule.h>

enum
{
  PROP_PARAMETER = 1,
  PROP_PARAMETER_KV,
  PROP_NAME,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = {
  NULL,
};

typedef struct
{
  gchar *parameter;
  GHashTable *parameter_table;
  gchar *name;
} ChatbotModulePrivate;

static void chatbot_module_initable_init (GInitableIface *iface);

G_DEFINE_TYPE_EXTENDED (ChatbotModule, chatbot_module, G_TYPE_OBJECT, 0,
                        G_ADD_PRIVATE (ChatbotModule) G_IMPLEMENT_INTERFACE (
                            G_TYPE_INITABLE, chatbot_module_initable_init));

static void
chatbot_module_initable_init (GInitableIface *iface)
{
}

static void
chatbot_module_set_property (GObject *object, guint property_id,
                             const GValue *value, GParamSpec *pspec)
{
  ChatbotModulePrivate *priv
      = chatbot_module_get_instance_private (CHATBOT_MODULE (object));

  switch (property_id)
    {
    case PROP_PARAMETER:
      g_clear_pointer (&priv->parameter, g_free);
      priv->parameter = g_value_dup_string (value);
      break;
    case PROP_PARAMETER_KV:
      g_clear_pointer (&priv->parameter_table, g_hash_table_unref);
      priv->parameter_table = g_value_dup_boxed (value);
      break;
    case PROP_NAME:
      g_clear_pointer (&priv->name, g_free);
      priv->name = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
chatbot_module_get_property (GObject *object, guint property_id, GValue *value,
                             GParamSpec *pspec)
{
  ChatbotModulePrivate *priv
      = chatbot_module_get_instance_private (CHATBOT_MODULE (object));

  switch (property_id)
    {
    case PROP_PARAMETER:
      g_value_set_string (value, priv->parameter);
      break;
    case PROP_PARAMETER_KV:
      g_value_set_boxed (value, priv->parameter_table);
      break;
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
chatbot_module_dispose (GObject *object)
{
  ChatbotModulePrivate *priv
      = chatbot_module_get_instance_private (CHATBOT_MODULE (object));

  g_clear_pointer (&priv->parameter_table, g_hash_table_unref);

  G_OBJECT_CLASS (chatbot_module_parent_class)->dispose (object);
}

static void
chatbot_module_finalize (GObject *object)
{
  ChatbotModulePrivate *priv
      = chatbot_module_get_instance_private (CHATBOT_MODULE (object));

  g_clear_pointer (&priv->name, g_free);
  g_clear_pointer (&priv->parameter, g_free);

  G_OBJECT_CLASS (chatbot_module_parent_class)->finalize (object);
}

static void
chatbot_module_constructed (GObject *object)
{
  ChatbotModule *module = CHATBOT_MODULE (object);
  ChatbotModulePrivate *priv = chatbot_module_get_instance_private (module);

  gchar **kv_array;
  kv_array = g_strsplit (priv->parameter, ":", -1);

  for (gchar **iter = kv_array; *iter != NULL; iter++)
    {
      gchar **kv = g_strsplit (*iter, "=", 2); // "k=v" into {k, v}
      if (!g_hash_table_insert (priv->parameter_table, kv[0], kv[1]))
        g_warning ("Failed to insert parameter \"%s\".", kv[0]);
      g_strfreev (kv);
    }

  g_strfreev (kv_array);
  G_OBJECT_CLASS (chatbot_module_parent_class)->constructed (object);
}

static void
chatbot_module_class_init (ChatbotModuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = chatbot_module_set_property;
  object_class->get_property = chatbot_module_get_property;
  object_class->dispose = chatbot_module_dispose;
  object_class->finalize = chatbot_module_finalize;
  object_class->constructed = chatbot_module_constructed;

  /**
   * Module:_parameter:
   *
   * raw module parameter to construct [property@Module:parameter]
   */
  properties[PROP_PARAMETER] = g_param_spec_string (
      "raw_parameter", "raw_parameter", "raw parameter string", "",
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);

  /**
   * Module:parameter:
   *
   * Key-Value based module parameter.
   */
  properties[PROP_PARAMETER_KV]
      = g_param_spec_boxed ("parameter", "parameter", "parameter key-value",
                            G_TYPE_HASH_TABLE, G_PARAM_READABLE);

  /**
   * Module:name:
   *
   * Name of the module.
   */
  properties[PROP_NAME]
      = g_param_spec_string ("name", "name", "module name", NULL,
                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
chatbot_module_init (ChatbotModule *module)
{
  ChatbotModulePrivate *priv = chatbot_module_get_instance_private (module);

  priv->parameter_table
      = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}

/**
 * chatbot_module_new:
 * @type GType which is module implementation
 * @parameter module parameter
 * @error location to store the error
 *
 * Create instance from GType.
 *
 * Returns: newly created instance
 */
ChatbotModule *
chatbot_module_new (GType type, const gchar *parameter, GError **error)
{
  return g_initable_new (type, NULL, error, "parameter", parameter, NULL);
}

/**
 * chatbot_module_get_name: (get-property name)
 *
 * Get the module name.
 *
 * Returns: [property@Module:name]
 */
const gchar *
chatbot_module_get_name (ChatbotModule *module)
{
  ChatbotModulePrivate *priv = chatbot_module_get_instance_private (module);
  g_return_val_if_fail (CHATBOT_IS_MODULE (module), NULL);
  return priv->name;
}
