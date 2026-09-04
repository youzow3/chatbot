test_common_src = files(
  'chatbot-test-error.h',
  'chatbot-test-error.c',
  'chatbot-test-data.h',
  'chatbot-test-data.c',
  'chatbot-test-language-model.h',
  'chatbot-test-language-model.c',
  'chatbot-test-agent.h',
  'chatbot-test-agent.c'
)
test_common = library('chatbot-test-common', test_common_src, dependencies: [chatbot_dep])

units = [
  'data',
  'message',
  'message-array',
  'text-plain'
]

foreach u: units
  e = executable(u, files('chatbot-' + u + '.c'), link_with: [test_common], dependencies: [chatbot_dep])
  test(u, e)
endforeach
