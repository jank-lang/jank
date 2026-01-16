# Casting between native types
jank has two primary means of casting between native types.

1. `cpp/cast`
2. `cpp/unsafe-cast`

They both have the same syntax, but they perform different actions.

## `cpp/cast`
This is the most common style of cast which you'll see in jank. It closely maps
to C++ `static_cast`, but it has additional functionality to support jank's
[trait conversions](./native-values.html#trait-convertible). This will allow you
to cast to/from jank runtime object and supported native C++ values.

```clojure
(fn [o]
  (let [f #cpp 3.14
        ; Normal static_cast support.
        i (cpp/cast cpp/int f)
        ; Explicit trait conversion, since `o` is a jank object.
        oi (cpp/cast cpp/int o)])
```

## `cpp/unsafe-cast`
When `cpp/cast` is not enough, jank supports a more cutting cast which is the
equivalent of C-style casting in C++. This is a combination of `static_cast`,
`reinterpret_cast`, and `const_cast`. For example, if you need to cast between unrelated
pointer types, `cpp/cast` will not work, but `cpp/unsafe-cast` will. However,
note that `cpp/unsafe-cast` does not support trait conversions. It is solely
dedicated to native type construction and reinterpretation.

```clojure
(let [s #cpp "meow"
      us (cpp/cast (cpp/type "unsigned char*") s)])
```
