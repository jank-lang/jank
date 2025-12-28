# Differences from Clojure

## clojure.core
* Baked into the compiler
* No nested require support
* No `import`
* `(hash-map)` returns a hash map, not an array map
* `aget` is a special form
* `aset` is a macro
* `keyword` is more strict about valid inputs

## Object Model
* No stable boxes for small integers (the JVM pre-allocates `1`, `2`, `3`, etc)
* String is UTF-8
* No inheritance
* No records
* No protocols
* Sequences
  * In-place operations (`fresh-seq`, `next-in-place`)

## Interop
See TODO: Interop.

## Compilation Model
* Global compilation artifacts
* Source-only distribution (i.e. class files)
  * Git deps are an exception here; if someone commits a .o file into a git dep
    on your module path, jank will load it
* Dependencies are AOT compiled by default
* Multi-threaded dependency compilation
* Module path vs class path

## Math
* Division by integer 0 is UB
* Division by floating point 0 is well defined
