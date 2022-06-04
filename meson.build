project(
  'jank',
  'cpp',
  version : 'snapshot',
  default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++17', 'cpp_debugstl=true']
)

project_inc = include_directories('include', 'lib/magic_enum/include', 'lib/immer')
project_src = files([
  'src/jank/util/mapped_file.cpp',
  'src/jank/read/lex.cpp',
  'src/jank/read/parse.cpp',
  'src/jank/runtime/seq.cpp',
  'src/jank/runtime/memory_pool.cpp',
  'src/jank/runtime/object.cpp',
  'src/jank/runtime/context.cpp',
  'src/jank/runtime/ns.cpp',
  'src/jank/runtime/var.cpp',
  'src/jank/runtime/obj/number.cpp',
  'src/jank/runtime/obj/function.cpp',
  'src/jank/runtime/obj/symbol.cpp',
  'src/jank/runtime/obj/list.cpp',
  'src/jank/runtime/obj/vector.cpp',
  'src/jank/runtime/obj/map.cpp',
  'src/jank/runtime/obj/set.cpp',
  'src/jank/runtime/obj/string.cpp',
  'src/jank/runtime/behavior/callable.cpp',
  'src/jank/analyze/processor.cpp',
  'src/jank/evaluate/context.cpp',
])

add_global_arguments(
  [
    '-Wfloat-equal', '-Wuninitialized',
    '-Wswitch-enum', '-Wnon-virtual-dtor', '-Wold-style-cast',
    '-Wno-gnu-case-range', '-Wno-error=#warnings',
    '-fprofile-instr-generate', '-fcoverage-mapping',
  ],
  language : 'cpp'
)
# TODO: Disable these in release.
add_global_arguments([
  '-fno-omit-frame-pointer', '-fno-optimize-sibling-calls',
  '-fstack-protector-strong'
  ],
  language : 'cpp'
)

# TODO: Release flags
# -O3 -march=native -DNDEBUG -flto

threads_dep = dependency('threads')
boost_dep = dependency('boost')
executable(
  'jank',
  project_src + ['src/main.cpp'],
  dependencies : [threads_dep, boost_dep],
  link_args : ['-fprofile-instr-generate', '-fcoverage-mapping'],
  include_directories : project_inc,
  install : true
)

subdir('test')
