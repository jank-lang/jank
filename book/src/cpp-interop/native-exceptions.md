# Throwing and catching exceptions
jank integrates tightly into C++'s exception model. C++ allows values of any
type to be thrown, caught, and rethrown. So does jank.

## Throwing and catching jank objects
When you throw a value from jank, regardless of its type, the value will be
type-erased into an `object_ref`. It doesn't matter if you throw a keyword or a
hash map or a string, or any other jank runtime object, you catch it as an
`object_ref`. For example:

```clojure
(defn -main [& args]
  (try
    (throw :ok!)
    (catch cpp/jank.runtime.object_ref e
      (println :caught e))))
```

## Throwing and catching native values
Many C++ libraries will throw values which are not jank runtime objects. A very
common type to throw is a class derived from `std::exception`. From jank, we can
catch any C++ type and, just like in C++, we can catch exceptions via their base
type as well.

In this example, calling `.at` on a `std::vector`, with an invalid index, will
throw a `std::out_of_range` exception, which derives from `std::exception`. We
can catch the exception by the base type and then rely on the virtual `.what`
member function to get the exception message.
```clojure

(let [v ((cpp/type "std::vector<int>"))]
  (try
    ; This will throw.
    (.at v #cpp 0)
    (catch cpp/std.exception e
      (println :caught (.what e)))))
```

> [!NOTE]
> jank doesn't yet support providing native values to `(throw ...)`, but it will
> soon.
>
> Also, jank doesn't yet support the equivalent of C++'s catch all, which
> catches any exception type, but doesn't provide the value. We will support
> this, too.
