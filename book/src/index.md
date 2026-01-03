# Welcome to the jank alpha!
*by Jeaye Wilkerson, without LLMs, with feedback from the jank community.*

This book is written for jank's alpha release. It is incomplete, although its
incompleteness matches jank's. It's still early to jump into jank, but your
time and patience is welcome.

> [!IMPORTANT]
> jank is alpha quality software. It will crash. It will leak. It will be slow.
> There are huge areas of functionality which haven't been implemented. Your
> help getting us past this stage is greatly appreciated.

## What is jank?
jank is a general purpose programming language. It's a dialect of Clojure, which
is itself a dialect of Lisp. jank is functional-first, but it supports adhoc
mutations and effects. All data structures are persistent and immutable by
default and jank, following Clojure's design, provides mechanisms for safe
mutations for easy concurrency.

Beyond Clojure, jank is brethren to C++ and it can reach into C++ arbitrarily to
both access and define new C++ types, functions, and templates, at runtime. This
is done by JIT (just in time) compiling C++, using Clang and LLVM. The result is
that you can write Clojure code which can access C and C++ libraries trivially.

For more details on jank's status, please read the [foreword](foreword.md).
