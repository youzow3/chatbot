project('chatbot', 'c')

gnome = import('gnome')

glib_dep = dependency('glib-2.0')
gobject_dep = dependency('gobject-2.0')
gmodule_dep = dependency('gmodule-2.0')
gio_dep = dependency('gio-2.0')

chatbot_src = files(
  'chatbot/chatbot-language-model.h',
  'chatbot/chatbot-language-model.c',
  'chatbot/chatbot-tool.h',
  'chatbot/chatbot-tool.c',
  'chatbot/chatbot-tool-manager.h',
  'chatbot/chatbot-tool-manager.c',
)

chatbot_inc = 'chatbot/'

libchatbot = library('chatbot', chatbot_src, dependencies: [gobject_dep, gio_dep])

chatbot_gir = gnome.generate_gir(
  libchatbot,
  sources: chatbot_src,
  namespace: 'Chatbot',
  nsversion: '1.0',
  identifier_prefix: 'Chatbot',
  symbol_prefix: 'chatbot',
  export_packages: 'chatbot',
  dependencies: [gobject_dep, gio_dep],
  includes: ['GObject-2.0', 'Gio-2.0'],
  header: 'chatbot/chatbot.h',
  install: true
)

gi_docgen = find_program('gi-docgen')
if gi_docgen.found()
  chatbot_docs = custom_target(
    'chatbot-docs', build_by_default: true,
    command: ['gi-docgen', 'generate', '-C', '@INPUT0@', '--output-dir', '@OUTPUT@', '@INPUT1@'],
    input: ['chatbot.toml', chatbot_gir[0]], output: ['chatbot-1.0']
  )
endif

chatbot_dep = declare_dependency(link_with: libchatbot, include_directories: [chatbot_inc], dependencies: [gobject_dep, gio_dep])

cli_src = files(
  'cli/main.c'
)

executable('cli', cli_src, dependencies: [gmodule_dep, gio_dep, chatbot_dep])
