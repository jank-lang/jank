project(
  'jank',
  'cpp',
  version : 'snapshot',
  default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++17']
)

project_inc = include_directories('include/cpp')
project_src = files([
  'src/cpp/jank/util/mapped_file.cpp',
  'src/cpp/jank/read/lex.cpp',
  'src/cpp/jank/read/parse.cpp',
  'src/cpp/jank/runtime/seq.cpp',
  'src/cpp/jank/runtime/memory_pool.cpp',
  'src/cpp/jank/runtime/object.cpp',
  'src/cpp/jank/runtime/context.cpp',
  'src/cpp/jank/runtime/ns.cpp',
  'src/cpp/jank/runtime/var.cpp',
  'src/cpp/jank/runtime/obj/number.cpp',
  'src/cpp/jank/runtime/obj/function.cpp',
  'src/cpp/jank/runtime/obj/symbol.cpp',
  'src/cpp/jank/runtime/obj/keyword.cpp',
  'src/cpp/jank/runtime/obj/list.cpp',
  'src/cpp/jank/runtime/obj/vector.cpp',
  'src/cpp/jank/runtime/obj/map.cpp',
  'src/cpp/jank/runtime/obj/set.cpp',
  'src/cpp/jank/runtime/obj/string.cpp',
  'src/cpp/jank/runtime/behavior/callable.cpp',
  'src/cpp/jank/analyze/processor.cpp',
  'src/cpp/jank/evaluate/context.cpp',
  'src/cpp/jank/codegen/context.cpp',
])

add_global_arguments(
  [
    '-Wfloat-equal', '-Wuninitialized',
    '-Wswitch-enum', '-Wnon-virtual-dtor', '-Wold-style-cast',
    '-Wno-gnu-case-range', '-Wno-error=#warnings', '-Wno-gnu-include-next'
  ],
  language : 'cpp'
)

add_global_arguments(['-DLLVM_PATH="' + get_option('llvm_root_path') + '"'], language : 'cpp')

if get_option('jank_build_type') == 'release'
  option_overrides = [
    'buildtype=release',
    'optimization=3',
    'cpp_debugstl=false',
    'b_lto=true',
    'b_ndebug=true',
    'strip=true',
  ]
  link_args = []
  add_global_arguments([
    ],
    language : 'cpp'
  )
elif get_option('jank_build_type') == 'debug'
  option_overrides = [
    'buildtype=debug',
    'optimization=g',
    'cpp_debugstl=true',
    'b_lto=false',
    'b_ndebug=false',
    'strip=false',
  ]
  link_args = ['-fprofile-instr-generate', '-fcoverage-mapping']
  add_global_arguments([
    '-fno-omit-frame-pointer', '-fno-optimize-sibling-calls',
    '-fstack-protector-strong',
    '-fprofile-instr-generate', '-fcoverage-mapping'],
    language : 'cpp'
  )
else
  error('invalid build mode: ' + get_option('jank_build_mode'))
endif

threads_dep = dependency('threads')
boost_dep = dependency('Boost')
magic_enum_dep = dependency('magic_enum')
immer_dep = dependency('Immer')
folly_dep = dependency('folly')
cling_dep = declare_dependency(
  include_directories : include_directories(
    get_option('cling_include_path'),
    get_option('llvm_include_path'),
  ),
  link_args : ['-L' + get_option('cling_lib_path'), '-lcling', '-Wl,--export-dynamic', '-rdynamic'],
)
executable(
  'jank',
  project_src + ['src/cpp/main.cpp'],
  dependencies : [threads_dep, boost_dep, magic_enum_dep, immer_dep, folly_dep, cling_dep],
  link_args : link_args,
  include_directories : project_inc,
  override_options : option_overrides,
  install : true
)

subdir('test/cpp')
