test_inc = include_directories('../lib/doctest')
test_src = [
  'main.cpp',
  'jank/read/lex.cpp',
  'jank/read/parse.cpp',
  'jank/runtime/detail/list_type.cpp',
]
unit = executable(
  'jank-unit-tests',
  project_src + test_src,
  include_directories : [project_inc] + [test_inc],
  dependencies : threads_dep
)
test('unit tests', unit)
