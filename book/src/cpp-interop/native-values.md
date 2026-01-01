# Working with native values
C++ values, references, and pointers can be used directly within jank. However,
there are some limitations due to how C++'s object model works. The primary
factor here is that C++ has no base object type for all classes and structs.
Each top-level type is standalone. This is different from the JVM, CLR, and JS
environments where there is a base `Object` for all class types. This means that
we need to take some extra steps in order to store arbitrary C++ values within
the jank runtime. There are a few ways this can be done and jank tries to make
this as easy as possible.

## Trait-convertible
Firstly, some C++ types are automatically and implicitly convertible to/from
jank objects. These include all C++ intrinsic intregral types, bools, C strings,
and even some C++ standard libary types like `std::string`. For these types,
the jank compiler will detect if conversion is necessary and will implicitly
handle conversions as needed. For example, let's take a look at this jank code
which calls a C++ function which operates on `std::string`.

```clojure
(cpp/raw "std::string to_upper(std::string const &s)
          {
            std::string ret;
            for(auto const c : s)
            { ret += ::toupper(c); }
            return ret;
          }")

(defn to-upper [o]
  (let [upper (cpp/to_upper o)]
    upper))
```

In this code, `o` is a `jank::runtime::object_ref`. This is basically like
Clojure's `Object` type. It's a garbage collected, type-erased value. When the
jank compiler analyzes the call to `(cpp/to_upper o)`, it resolves that
`to_upper` expects a `std::string` and that there is a conversion trait for it.
So the jank compiler will automatically handle converting from `object_ref` into
a `std::string`. On the other side, `upper` is a `std::string`, which is the
result of `to_upper`. However, when we return it from the `let`, the jank
compiler sees that it can implicitly create an `object_ref` for us, so there's
nothing we need to do.

## Non-trait-convertible
Aside from the built-in supported trait conversions, every other C++ type will
not be convertible. If you try to pass such a value as a jank function argument
or if you try to return such a value from a jank function, you will get a
compiler error. For example, given this source:

```clojure
(cpp/raw "struct person
          { std::string name; };")

(defn create-person [name]
  (let [p (cpp/person (cpp/cast cpp/std.string name))]
    p))
```

If we try to run this file, we'll get a compiler error telling us that we can't
return a value of type `person` from our function, since it's not convertible to
a jank runtime object.

```
$ jank run person.jank
─ analyze/invalid-cpp-conversion ────────────────────────────
error: This function is returning a native object of type
       'person', which is not convertible to a jank runtime
       object.

─────┬───────────────────────────────────────────────────────
     │ test.jank
─────┼───────────────────────────────────────────────────────
  2  │           { std::string name; };")
  3  │
  4  │ (defn create-person [name]
     │ ^ Expanded from this macro.
─────┴───────────────────────────────────────────────────────
```

## Implementing your own trait
To build on the `person` type defined above, we could extend the conversion
trait to teach jank how to convert to/from `person` and jank maps. This does
require C++ template metaprogramming, which is an advanced concept that's only
intended for C++ developers who're using jank.

```clojure

(cpp/raw "struct person
          { std::string name; };


          namespace jank::runtime
          {
            template <>
            struct convert<person>
            {
              static obj::keyword_ref name_kw;

              static obj::persistent_hash_map_ref into_object(person const &p)
              {
                return obj::persistent_hash_map::create_unique(std::make_pair(name_kw, make_box(p.name)));
              }

              static person from_object(object_ref const o)
              {
                auto const name{ try_object<obj::persistent_string>(get(o, name_kw)) };
                return person{ name->data };
              }
             };

             obj::keyword_ref convert<person>::name_kw{ __rt_ctx->intern_keyword(\"name\").expect_ok() };
          }

          ")

(defn create-person [name]
  (let [p (cpp/person (cpp/cast cpp/std.string name))]
    p))

(println (create-person "foo"))
```

Now, if we're to run this, we can see that the `person` was implicitly converted
into a jank hash map.

```bash
$ jank run person.jank
{:name foo}
```

## Opaque boxes
