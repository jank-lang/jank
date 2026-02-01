# Differences from Clojure
jank is meant to be Clojure, but Clojure itself has no specification. There are
differences between Clojure JVM, ClojureScript, Clojure CLR, ClojureDart, and
others. Part of being a Clojure is embracing one's host and being transparent
about it. This is where most of the differences come into play.

jank does not try to hide its C++ host. That would defeat the point of being
Clojure.

## Command line
* You will find no Clojure CLI `-X:foo` syntax here
* When using `jank run` or `jank run-main`, `--` is needed to separate args for
  jank from args for your program
    * Example: `jank -I include run test.jank -- a b c`

## Parser
* No load operation for `data_readers.(cljc|jank)` at start-up to extend
  supported tags
* Only lexer errors and unbalanced forms are rejected in unsupported reader
  conditionals

## clojure.core
* Baked into the `jank` binary, not shipped separately
* No nested `require` support (same as ClojureScript)
* No `import`
* `(hash-map)` returns a hash map, not an array map
* `aget` is a special form
* `aset` is a macro
* `keyword` is more strict about valid inputs
* `future` can only forward exceptions which are `std::exception` or `object_ref`
  * Other exceptions will be forwarded as `"Unknown exception"`
* `future-cancel` returns `nil`, not the result of the cancellation
* `future-cancelled?` always returns `false` on macOS, since there is no
  reliable way to check this with pthread

## Object model
* No stable boxes for small integers (the JVM pre-allocates `1`, `2`, `3`, etc)
* `persistent_string` is expected to be UTF-8
* No inheritance (currently)
* No records (yet)
* No protocols (yet)
* Sequences
  * Support for in-place operations (`fresh-seq`, `next-in-place`)

## Compilation model
* Source-only distribution
  * `.o` files found in JARs will not be used
  * Git deps are an exception here; if someone commits a .o file into a git dep
    on your module path, jank will load it
* jank uses the term "module path" instead of "class path"
  * We don't have `.class` files
  * A module is backed by either a `.jank`, `.cljc` or `.cpp` source file,
    optionally with a `.o` file cached for quick loading

## Math
* Division by integer `0` is undefined behavior
* Division by floating point `0.0` is well defined
