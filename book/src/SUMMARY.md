# The jank Programming Language

[Welcome to the jank alpha!](index.md)

[Foreword](foreword.md)

* [Getting Started](getting-started/index.md)
  * [Installation](getting-started/01-installation.md)
  * [Hello, world!](getting-started/02-hello-world.md)
  * [Hello, Leiningen!](getting-started/03-hello-leiningen.md)

<!--
TODO:
  Samples of jank
  Reading error messages
  .jank, .cljc, .cpp files
    All modules need to intern the ns
  nREPL support
  Casting between types
  AOT compiling programs (and distributing them)
  Printing codegen
  Known issues
-->

* [Reaching into C++](cpp-interop/index.md)
  * [Embedding raw C++](cpp-interop/cpp-raw.md)
  * [Bringing in native libraries](cpp-interop/native-libs.md)
  * [Working with native values](cpp-interop/native-values.md)
  * [Working with native types](cpp-interop/native-types.md)
  * [Working with native functions](cpp-interop/native-functions.md)
  * [The reasoning for the cpp namespace](cpp-interop/cpp-ns.md)

* [Differences from Clojure](differences-from-clojure.md)
* [For C++ Developers](for-cpp-developers.md)

* [Troubleshooting](troubleshooting/index.md)
  * [Checking jank's health](troubleshooting/health-check.md)
  * [How to get a stack trace](troubleshooting/stack-trace.md)
  * [Where to get help](troubleshooting/getting-help.md)
