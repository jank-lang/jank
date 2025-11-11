# Differences from Clojure

## Reader

## Runtime
* `(fn foo [] foo)` will return a different instance than the invoking

## clojure.core
* Baked into the compiler
* No nested require support
* No `import`
* `(hash-map)` returns a hash map
* `aget` is a special
* `aset` is a macro
* `keyword` is more strict about valid inputs

## Object Model
* No stable boxes for small integers
* String is UTF-8
* No inheritance
* No records
* No protocols
* Sequences
  * In-place operations (`fresh-seq`, `next-in-place`)

## Interop

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
