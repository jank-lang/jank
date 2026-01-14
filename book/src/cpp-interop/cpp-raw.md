# Embedding raw C++
jank has a special `cpp/raw` form which accepts a single string containing
literal C++ code. This can be used for bringing in pretty much anything. For
example, we can use this to include header files.

```clojure
(cpp/raw "#include <fstream>")
```

jank will always compile the included C++ source in a global scope, even if you
put the `cpp/raw` form within a nested scope, such as within a function or a
`let`. For example, this code will have the same effect, even if this function
is never called.

```clojure
(defn foo []
  (cpp/raw "#include <fstream>"))
```

The `cpp/raw` form always evaluates to `nil`. At runtime, `foo` will do nothing
but return `nil`, since the JIT compilation is where the effect of `cpp/raw`
actually happens.

## A helpful idiom
Hopefully this becomes a less common idiom simply by not being needed, but for
now it's common enough. If you run into issues trying to access a member, call a
function, etc using normal C++ interop, you can write a wrapper in `cpp/raw`
which will do the trick. For example, let's say we have the following code.

```clojure
(let [s (cpp/std.string)
      ; Let's say that this interop call isn't compiling correctly, due to a
      ; jank bug.
      size (cpp/.size s)]
  (println "The size is" size))
```

You can work around this issue by defining a helper function which does the C++
work for you. In this case, we could do the following.

```clojure
(cpp/raw "size_t get_string_size(std::string const &s)
          { return s.size(); }")

(let [s (cpp/std.string)
      size (cpp/get_string_size s)]
  (println "The size is" size))
```


Of course, if you need to use this, please also report a bug on jank's Github
which describes what you tried to do and why it didn't work.
