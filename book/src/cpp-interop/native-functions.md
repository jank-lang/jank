# Working with native functions

## Global functions
C++ has a huge range of function scenarios and jank tries to capture them all.
The simplest case is global functions, as well as static member functions. This
applies to both C and C++ functions. In order to call these, just take the fully
qualified name of the function and replace `::` with `.`. For example:

* `std::rand` becomes `cpp/rand`
* `std::this_thread::get_id()` becomes `cpp/std.this_thread.get_id`

For example, we can use the C functions `srand`, `time`, and `rand` to seed the
pseudo-random number generator with the current time and then get a number.

```clojure
(defn -main [& args]
  (cpp/srand (cpp/time cpp/nullptr))
  (println "rand:" (cpp/rand)))
```

## Overload resolution
Once we get out of C land and into C++ territory, the function name alone
doesn't necessarily make it unique. C++ functions can be overloaded with
different arities and different parameter types. jank will resolve each function
call at compile-time. There is no runtime reflection. If a call can't be
resolved to a known overload, or is ambiguous between multiple overloads, jank
will raise a compiler error.

For example, the
[std::to_string](https://en.cppreference.com/w/cpp/string/basic_string/to_string)
function has many different overloads. Here, we specifically create `i` to be an
`int`, so overload resolution can happen.

```clojure
(defn -main [& args]
  (let [i #cpp 42
        s (cpp/std.to_string i)]
    s))
```

However, if we try to rely on implicit trait conversions, or we pass an unsupported
type, we'll get a compiler error.

```clojure
(defn -main [& args]
  (let [i 42
        s (cpp/std.to_string i)]
    s))
```

```bash
$ jank run test.jank
─ analyze/invalid-cpp-function-call ───────────────────────────────────────────
error: No normal overload match was found. When considering automatic trait
       conversions, this call is ambiguous.

─────┬─────────────────────────────────────────────────────────────────────────
     │ test.jank
─────┼─────────────────────────────────────────────────────────────────────────
  1  │ (defn -main [& args]
  2  │   (let [i 42
     │   ^ Expanded from this macro.
  3  │         s (cpp/std.to_string i)]
     │            ^^^^^^^^^^^^^^^^^ Found here.
─────┴─────────────────────────────────────────────────────────────────────────
```

We could opt into a specific conversion, and thus a specific overload, by using `cpp/cast`.

```clojure
(defn -main [& args]
  (let [i 42
        s (cpp/std.to_string (cpp/cast cpp/int i))]
    s))
```

## Member functions
Member functions can be accessed using the `cpp/.foo` syntax. For example, let's
convert a jank object to a `std::string` and then see if it's empty.

```clojure
(defn empty? [o]
  (let [s (str o)
        native-s (cpp/cast cpp/std.string s)]
    (cpp/.empty native-s)))
```

Also note that member functions can be called through a pointer to the native
object, without the need for an explicit dereference.

## Arbitrary callables
In C++, we also deal with pointers to functions and custom types which implement
the call operator. jank supports both of these scenarios using the normal call
syntax. For example, we can implement our own callable which captures some data
and then returns it when called.

```clojure
(cpp/raw "struct call_me
          {
            jank::runtime::object_ref data;

            jank::runtime::object_ref operator()()
            { return data; }
          };")

(defn -main [& args]
  (let [f (cpp/call_me. "meow")]
    (f)))
```

## Operators
C++ operators are special language features for primitives, but they can also be
overloaded for custom types. Their semantics are much more complicated than
Clojure's function calls, but basically all of them are available under the
`cpp/` namespace within jank.

> [!NOTE]
> C++20 does operator rewriting for comparison operators, to use the `<=>`
> spaceship operator, or perhaps others. jank doesn't currently support this.
> If you're porting C++ code to jank which fails to find the appropriate
> operator, chances are that operator never existed and Clang used rewriting to
> use a different operator instead.
