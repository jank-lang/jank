project(
  'jank',
  'cpp',
  version : 'snapshot',
  default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++17']
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
    '-fprofile-instr-generate', '-fcoverage-mapping',
    ],
    language : 'cpp'
  )
else
  error('invalid build mode: ' + get_option('jank_build_mode'))
endif

threads_dep = dependency('threads')
boost_dep = dependency('boost')
folly_dep = declare_dependency(
  link_args : ['-lfolly', '-lglog'],
)
cling_dep = declare_dependency(
  include_directories : include_directories(
    get_option('cling_include_path'),
    get_option('llvm_include_path'),
  ),
  link_args : ['-L' + get_option('cling_lib_path'), '-lcling'],
)
executable(
  'jank',
  project_src + ['src/main.cpp'],
  dependencies : [threads_dep, boost_dep, folly_dep, cling_dep],
  link_args : link_args,
  include_directories : project_inc,
  override_options : option_overrides,
  install : true
)

subdir('test')
