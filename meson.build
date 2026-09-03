project('chatbot', 'c')

gnome = import('gnome')

glib_dep = dependency('glib-2.0')
gobject_dep = dependency('gobject-2.0')
gmodule_dep = dependency('gmodule-2.0')
gio_dep = dependency('gio-2.0')
json_glib_dep = dependency('json-glib-1.0')

chatbot_src = files(
    'chatbot/chatbot-error.h',
    'chatbot/chatbot-error.c',
    'chatbot/chatbot-module.h',
    'chatbot/chatbot-module.c',
    'chatbot/chatbot-language-model.h',
    'chatbot/chatbot-language-model.c',
    'chatbot/chatbot-trainer.h',
    'chatbot/chatbot-trainer.c',
    'chatbot/chatbot-data.h',
    'chatbot/chatbot-data.c',
    'chatbot/chatbot-text-plain.h',
    'chatbot/chatbot-text-plain.c',
    'chatbot/chatbot-message-role.h',
    'chatbot/chatbot-message-role.c',
    'chatbot/chatbot-message.h',
    'chatbot/chatbot-message.c',
    'chatbot/chatbot-message-array.h',
    'chatbot/chatbot-message-array.c',
    'chatbot/chatbot-tool.h',
    'chatbot/chatbot-tool.c',
    'chatbot/chatbot-agent.h',
    'chatbot/chatbot-agent.c',
    'chatbot/chatbot-multi-agent.h',
    'chatbot/chatbot-multi-agent.c'
)

chatbot_inc = 'chatbot/'

libchatbot = library(
    'chatbot',
    chatbot_src,
    dependencies: [gobject_dep, gio_dep, json_glib_dep],
)

chatbot_gir = gnome.generate_gir(
    libchatbot,
    sources: chatbot_src,
    namespace: 'Chatbot',
    nsversion: '2.0',
    identifier_prefix: 'Chatbot',
    symbol_prefix: 'chatbot',
    export_packages: 'chatbot',
    dependencies: [glib_dep, gobject_dep, gio_dep, json_glib_dep],
    includes: ['GLib-2.0', 'GObject-2.0', 'Gio-2.0', 'Json-1.0'],
    header: 'chatbot/chatbot.h',
    install: true,
)

gi_docgen = find_program('gi-docgen')
if gi_docgen.found()
    chatbot_docs = custom_target(
        'chatbot-docs',
        build_by_default: true,
        command: [
            'gi-docgen',
            'generate',
            '-C',
            '@INPUT0@',
            '--output-dir',
            '@OUTPUT@',
            '@INPUT1@',
        ],
        input: ['chatbot.toml', chatbot_gir[0]],
        output: ['chatbot-2.0'],
    )
endif

chatbot_dep = declare_dependency(
    link_with: libchatbot,
    include_directories: [chatbot_inc, './'],
    dependencies: [gobject_dep, gio_dep, json_glib_dep],
)

subdir('test')
