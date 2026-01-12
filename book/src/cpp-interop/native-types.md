# Working with native types

## Accessing C++ types
C++ types are available under the `cpp/` namespace, if you replace `::` with
`.`. For example, `std::string` becomes `cpp/std.string`. This also works for
type aliases.

## Complex literal types
If your C++ type is not representable using jank's interop syntax, due to
template arguments or other shenanigans, you can use `cpp/type` to provide the
complete type using an inline C++ string. For example, here's how we both grab
the type and construct it (call the type), to get a value. In this example, we
build a C++ ordered map from `std::string` to pointers to functions which take
in an `int` and return an `int`.

```clojure
(let [m ((cpp/type "std::map<std::string, int (*)(int)>"))]
  )
```

## Defining new types
There isn't yet a way to define new types using jank's syntax, but you can
always drop to `cpp/raw` to either include headers or define some C++ types
inline. Improved support for extending jank's object model with JIT (just in
time) compiled types will be coming in 2026.
