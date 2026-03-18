# Working with native types

## Accessing C++ types
C++ types are available within the `cpp` namespace, but you must replace `::` with
`.`. For example, `std::string` becomes `cpp/std.string`. This also works for
type aliases.

## Complex literal types
For complex types like template instantiations, pointers to members, and so on,
jank supports a C++ domain-specific language (DSL).
This is available implicitly when in type position, but it can be
explicitly requested by using the special `#cpp` tag. The documentation for this
DSL is [here](./dsl.md).

## Defining new types
There isn't yet a way to define new types using jank's syntax, but you can
always drop to `cpp/raw` to either include headers or define some C++ types
inline. Improved support for extending jank's object model with JIT (just in
time) compiled types will be coming soon.

```clojure
(cpp/raw "struct person
          {
            std::string name;
          };")
```
