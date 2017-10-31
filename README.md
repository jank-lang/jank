# The jank programming language [![Build Status](https://travis-ci.org/jeaye/jank.svg?branch=master)](https://travis-ci.org/jeaye/jank) [![codecov](https://codecov.io/gh/jeaye/jank/branch/master/graph/badge.svg)](https://codecov.io/gh/jeaye/jank)

jank is a compiled functional programming language with a strong, static type system, scope-based resource management (RAII), and a direct focus on generic, compile-time meta-programming using a powerful type-based template system, dependent types, and code-as-data macros.

With a focus on safe parallelism, jank has immutable, persistent data
structures.

Currently, jank aims to provide:

* A compiler targeting C++14
* An interactive REPL (command line and web-based)

## Editor support
There are syntax files for Vim available in the `vim` directory of the repository. You can add these to your runtime path using something like:

```viml
set runtimepath^=~/projects/jank/vim
set runtimepath^=~/projects/jank/vim/after
```

## License
jank is under a strict copyleft license; see the
[LICENSE](https://github.com/jeaye/jank/blob/master/LICENSE) file.
