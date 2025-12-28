# Bringing in native libraries
Ultimately, jank exists to combine Clojure and C++. Most non-trivial jank
programs will end up reaching into the C++ world for *something*. jank embraces
familiar concepts from the C++ world, since it's built entirely around Clang.

1. Include search paths
2. Preprocessor defines
3. Linker search paths
4. Libaries to link

## Using Leiningen
To bring in a C or C++ library, you will first need to `#include` the necessary
header files. In order to be able to include them, you need to tell jank (and
thus Clang) where to find them. This is done with your include search paths.

Next, you may need to define some preprocessor macros in order to use or
customize the library.

Finally, you may need to link to some libraries involved. This often includes
library search paths as well as library names, but library names can also be
absolute paths, if that's applicable to you.

> [!NOTE]
> The way native linking works is that you link to `my-lib`, but the linker
> will actually look for `libmy-lib.a` and `libmy-lib.so` (or `libmy-lib.dylib`
> on macOS). Do not put the full file name in `:linked-libraries` unless
> you're also specifying the absolute path.

```clojure
(defproject hello_lein "0.1-SNAPSHOT"
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "0.2"]]
  :middleware [leiningen.jank/middleware]
  :main hello-lein.main

  ; Look here.
  :jank {:include-dirs ["/usr/share/my-lib/include"
                        "third-party/foo/include"]
         :defines {"IMMER_HAS_LIBGC" "1"}
         :library-dirs ["/usr/share/my-lib/lib"]
         :linked-libraries ["my-lib"]}

  :profiles {:debug {:jank {:optimization-level 0}}
             :release {:jank {:optimization-level 2}}})
```

> [!NOTE]
> jank does not support C++20 modules right now. In fact, most C++ compilers
> have very poor support for C++20 modules right now. This may come, in the
> future, but it will only happen once the experience is sane.

## Using jank directly
jank exposes the same flags as Clang for includes, defines, and linked
libraries. They work in the same way, too. For all of these, you may add as many
as you need.

* Specify `-I <path>` to add a new include path
* Specify `-D FOO` or `-D FOO=bar` to add a new preprocessor define
* Specify `-L <path>` to add a new library path
* Specify `-l <lib>` to link to a library name
