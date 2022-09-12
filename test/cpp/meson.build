test_src = [
  'main.cpp',
  'jank/read/lex.cpp',
  'jank/read/parse.cpp',
  'jank/runtime/detail/list_type.cpp',
  'jank/runtime/context.cpp',
  'jank/analyze/expr/def.cpp',
  'jank/analyze/expr/var_deref.cpp',
  'jank/analyze/expr/function.cpp',
  'jank/analyze/expr/let.cpp',
  'jank/analyze/expr/primitive_literal.cpp',
  'jank/evaluate/context.cpp',
]

doctest_dep = dependency('doctest')

unit = executable(
  'jank-unit-tests',
  project_src + test_src,
  include_directories : [project_inc],
  link_args : link_args,
  override_options : option_overrides,
  dependencies : [threads_dep, boost_dep, magic_enum_dep, immer_dep, folly_dep, cling_dep, doctest_dep],
)
test('unit tests', unit)
