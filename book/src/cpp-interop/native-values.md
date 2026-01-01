# Working with native values
C++ values, references, and pointers can be used directly within jank. However,
there are some limitations due to how C++'s object model works. The primary
factor here is that C++ has no base object type for all classes and structs.
Each top-level type is standalone. This is different from the JVM, CLR, and JS
environments where there is a base `Object` for all class types. How this
manifests is that we need to take some extra steps in order to store arbitrary
C++ values within the jank runtime. There are a few ways this can be done and
jank tries to make this as easy as possible.

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

(defn to-upper [s]
  (let [upper (cpp/to_upper s)]
    upper))
```

## Non-trait-convertible

## Opaque boxes
