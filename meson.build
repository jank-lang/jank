project(
  'jank',
  'cpp',
  version : 'snapshot',
  default_options : ['warning_level=3', 'cpp_std=c++17', 'cpp_debugstl=true']
)

project_inc = include_directories('include', 'lib/magic_enum/include')
project_src = files([
  'src/cpp/jank/util/mapped_file.cpp',
  'src/cpp/jank/read/lex.cpp',
])

add_global_arguments(
  [
    '-Werror', '-Wfloat-equal', '-Wuninitialized',
    '-Wswitch-enum', '-Wnon-virtual-dtor', '-Wold-style-cast',
    '-Wno-gnu-case-range',
    '-Werror=format-security'
  ],
  language : 'cpp'
)
# TODO: Disable these in release.
add_global_arguments(
  ['-fno-omit-frame-pointer', '-fno-optimize-sibling-calls', '-fstack-protector-strong'],
  language : 'cpp'
)

# TODO: Release flags
# -O3 -march=native -DNDEBUG -flto

threads_dep = dependency('threads')
executable(
  'jank',
  project_src + ['src/cpp/main.cpp'],
  dependencies : threads_dep,
  include_directories : project_inc,
  install : true
)

subdir('test/cpp')
