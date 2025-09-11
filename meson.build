project('chatbot', 'c')

rwkv_tokenizer_proj = subproject('rwkv_tokenizer')
rwkv_tokenizer_dep = rwkv_tokenizer_proj.get_variable('rwkv_tokenizer_dep')

moduler_model_proj = subproject('moduler_model')
moduler_model_dep = moduler_model_proj.get_variable('moduler_model_dep')

glib_dep = dependency('glib-2.0')
gmodule_dep = dependency('gmodule-2.0')
gio_dep = dependency('gio-2.0')
onnxruntime_dep = dependency('libonnxruntime')

executable('chatbot', 'main.c', 'chatbot.c',
  dependencies: [glib_dep, gmodule_dep, gio_dep])
library('rwkv-module', 'rwkv.c',
  dependencies: [glib_dep, onnxruntime_dep,
    rwkv_tokenizer_dep, moduler_model_dep])
library('ish-module', 'ish.c', dependencies: [glib_dep, gio_dep])
