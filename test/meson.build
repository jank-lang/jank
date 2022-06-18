test_inc = include_directories('../lib/doctest')
test_src = [
  'main.cpp',
  'jank/read/lex.cpp',
  'jank/read/parse.cpp',
  'jank/runtime/detail/list_type.cpp',
  'jank/runtime/context.cpp',
  'jank/analyze/expr/def.cpp',
  'jank/analyze/expr/var_deref.cpp',
  'jank/evaluate/context.cpp',
]
unit = executable(
  'jank-unit-tests',
  project_src + test_src,
  include_directories : [project_inc] + [test_inc],
  link_args : link_args,
  override_options : option_overrides,
  dependencies : [threads_dep, boost_dep, cling_dep],
)
test('unit tests', unit)
