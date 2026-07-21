# The jank Programming Language

[Welcome to the jank alpha!](index.md)

[Foreword](foreword.md)

* [Getting Started](getting-started/index.md)
  * [Installation](getting-started/01-installation.md)
  * [Hello, world!](getting-started/02-hello-world.md)
  * [Hello, Leiningen!](getting-started/03-hello-leiningen.md)
  * [Hello, nREPL!](getting-started/04-hello-nrepl.md)

<!--
TODO:
  Samples of jank
  Reading error messages
  .jank, .cljc, .cpp files
    All modules need to intern the ns
  jank build system
    Guide: Packaging a native library
    Deprecate top-level project flags
    jank commons docs
  Library linking support
  Known issues
  Reference
   Special forms
   Error pages
-->

* [Reaching into C++](cpp-interop/index.md)
  * [Bringing in native libraries](cpp-interop/native-libs.md)
  * [Working with native values](cpp-interop/native-values.md)
  * [Working with native types](cpp-interop/native-types.md)
  * [Working with native functions](cpp-interop/native-functions.md)
  * [The C++ DSL](cpp-interop/dsl.md)
  * [Throwing and catching exceptions](cpp-interop/native-exceptions.md)
  * [Casting between native types](cpp-interop/cast.md)
  * [Embedding raw C++](cpp-interop/cpp-raw.md)
  * [The cpp namespace](cpp-interop/cpp-ns.md)

* [Working with projects](project/index.md)
  * [Testing](project/test.md)
  * [AOT compiling](project/aot.md)

* [The jank build system](jank-build/index.md)
  * [Build system overview](jank-build/overview.md)
  * [The build cache](jank-build/build-cache.md)
  * [Guide: Packaging a system library](jank-build/packaging-system-lib.md)
  * [Guide: Packaging a source library](jank-build/packaging-source-lib.md)

* [Differences from Clojure](differences-from-clojure.md)

* [Troubleshooting](troubleshooting/index.md)
  * [Checking jank's health](troubleshooting/health-check.md)
  * [Printing jank's IR or codegen](troubleshooting/printing.md)
  * [How to get a stack trace](troubleshooting/stack-trace.md)
  * [Where to get help](troubleshooting/getting-help.md)
  * [FAQ](troubleshooting/faq.md)

* [Developing jank](dev/index.md)
  * [IR reference](dev/ir.md)
