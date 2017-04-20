# The jank programming language [![Build Status](https://travis-ci.org/jeaye/jank.svg?branch=master)](https://travis-ci.org/jeaye/jank) [![codecov](https://codecov.io/gh/jeaye/jank/branch/master/graph/badge.svg)](https://codecov.io/gh/jeaye/jank)

jank is a compiled functional programming language with a strong, static type system, scope-based resource management (RAII), and a direct focus on generic, compile-time meta-programming using a powerful type-based template system, dependent types, and code-as-data macros.

With a focus on safe parallelism, jank has immutable, persistent data
structures.

Currently, jank aims to provide:

* A compiler targeting C++14
* An interactive REPL (command line and web-based)
