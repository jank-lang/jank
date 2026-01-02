# Working with native values
C++ values, references, and pointers can be used directly within jank. However,
there are some limitations due to how C++'s object model works. The primary
factor here is that C++ has no base object type for all classes and structs.
Each top-level type is standalone. This is different from the JVM, CLR, and JS
environments where there is a base `Object` for all class types. This means that
we need to take some extra steps in order to store arbitrary C++ values within
the jank runtime. There are a few ways this can be done and jank tries to make
this as easy as possible.

## Tagged literals
jank provides access to literal C++ values through the `#cpp` reader tag. Using
this will resolve to a C++ primitive, rather than a jank runtime object. It's
supported for the following literals:

1. Numbers (integers and floats)
2. Bools
3. Strings

For example:

```clojure
(let [i #cpp 0
      f #cpp 3.14159
      s #cpp "meow"]
  )
```

## Named literals
jank also has explicit support for `cpp/true`, `cpp/false`, and `cpp/nullptr`,
which all correspond to the C++ primitive.

## Member values
Member values can be accessed from a native object using the `cpp/.-foo` syntax.
For example, let's create a `person` and then pull out the name.

```clojure
(cpp/raw "struct person
          { std::string name; };")

(defn create-person [name]
  (let [p (cpp/person (cpp/cast cpp/std.string name))
        n (cpp/.-name p)]
    ))
```

Whenever a member is accessed, you will get a reference to it, not a copy. Also,
note that members can be access through a pointer to the native object, without
needing an explicit dereference.

```clojure
(defn create-person [name]
  (let [p (cpp/new cpp/person (cpp/cast cpp/std.string name))
        n (cpp/.-name p)]
    ))
```

## Trait-convertible
Some C++ types are automatically and implicitly convertible to/from
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
          }")

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

> [!NOTE]
> Although this works, consider moving this C++ into a header file and including
> it instead. Writing large amounts of C++ in `cpp/raw` strings doesn't scale very well.

## Opaque boxes
There is a performance cost to the convenience of implicit conversions. For pure
data, and trivial types, this may be preferred. However, if you want to store
something like a C++ database handle, which is managing network resources, a
thread pool, and other state, converting this to a jank runtime object is not
practical. In these cases, you can use an opaque box to pass the data through
the jank runtime instead.

Opaque boxes are jank runtime objects which basically store a `void*`, which is
an untyped native pointer. The key part here is that the data you store in the
opaque box must be a pointer. Since the opaque box could be stored in a
container, captured in a closure, or otherwise kept alive, it's important that
the data within is also dynamically allocated. However, opaque boxes track the
name of the type at compile-time and ensure that unboxing uses the correct type.
Given a hypothetical `my_db` C++ database library, boxing is done like this:

```clojure
(defn query! [db-box q]
  (let [; db-box is an object_ref
        ; db is a my_db.connection*
        db (cpp/unbox cpp/my_db.connection* db-box)]
    (cpp/.query db q)))

(defn -main [& args]
  (let [; db is a my_db.connection*
        db (cpp/new cpp/my_db.connection #cpp "localhost:5758")
        ; db-box is a opaque_box_ref
        db-box (cpp/box db)]
    ))
```

If you unbox the incorrect type, jank will surface a runtime error with helpful
source information describing the type that was in the opaque box and the type
you expected. For example, let's say we box a `connection*`, but we try to unbox
it as a `secure_connection*`.

```
❯ jank run test.jank
─ runtime/invalid-unbox ───────────────────────────────────────────────────────
error: This opaque box holds a 'my_db::connection*', but it was unboxed as a
       'my_db::secure_connection*'.

─────┬─────────────────────────────────────────────────────────────────────────
     │ test.jank
─────┼─────────────────────────────────────────────────────────────────────────
 21  │   (let [; db-box is an object_ref
 22  │         ; db is a my_db.connection*
 23  │         db (cpp/unbox cpp/my_db.secure_connection* db-box)]
     │             ^^^^^^^^^ Unboxed here.
     │ …
 28  │         db (cpp/new cpp/my_db.connection #cpp "localhost:5758")
 29  │         ; db-box is a opaque_box_ref
 30  │         db-box (cpp/box db)]
     │                 ^^^^^^^ Boxed here.
 31  │     (query! db-box "meow")))
 32  │
 33  │ (-main)
     │ ^^^^^^^ Used here.
─────┴─────────────────────────────────────────────────────────────────────────
```

## Complex literal values
If your C++ value is not representable using jank's interop syntax, due to
template arguments or other shenanigans, you can use `cpp/value` to provide the
complete value using an inline C++ string. For example, here's how we grab the
`npos` from a template instantiation:

```clojure
(let [m (cpp/value "std::basic_string<char>::npos")]
  )
```

No implicit boxing will happen here, unless you use this value in a way which
requires it. jank will give you a reference to the value you specified. If you
need a copy, you will need to manually do that.
