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

### Creating a native lib for testing
Let's make a new Leiningen project and a small C++ project within it.

```bash
$ lein new org.jank-lang/jank native-lib-tutorial
$ cd native-lib-tutorial
$ mkdir native-lib
$ cd native-lib
```

In this `native-lib` directory, let's make a small C++ library which uses libz
to compress the contents of a C++ string. Create a `compress.hpp` file with
these contents.

```cpp
#pragma once

#include <string>

namespace native_lib
{
  std::string compress(std::string const &input);
}
```

This is our header file. Now we can create a source file which implements our
function.

```cpp
#include <zlib.h>

#include <stdexcept>

#include "compress.hpp"

namespace native_lib
{
  std::string compress(std::string const &input)
  {
    if(input.empty())
    {
      return {};
    }

    auto const input_len{ input.size() };
    auto output_size{ compressBound(input_len) };

    std::string out;
    out.resize(output_size);

    auto const res{ ::compress((unsigned char *)out.data(),
                               &output_size,
                               (unsigned char *)input.data(),
                               input_len) };
    if(res != Z_OK)
    {
      throw std::runtime_error{ "compress failed: " + std::to_string(res) };
    }

    out.resize(output_size);
    return out;
  }
}
```

Our source file defines this `compress` function using zlib. We can now compile
this to a shared library so it can be used in jank.

```bash
# Linux.
$ clang++ -shared -o libcompress.so -lz compress.cpp
$ ls
compress.cpp  compress.hpp  libcompress.so

# macOS.
$ clang++ -shared -o libcompress.dylib -lz compress.cpp
$ ls
compress.cpp  compress.hpp  libcompress.dylib
```

### Linking to our native lib
Back in our jank project directory, let's try to use our new library. We'll
start by doing everything incorrectly, so we can see the types of errors jank
will raise and how to fix them.

To start with, let's update our `main.jank` to include our `compress.hpp` header
from our native lib.

```clojure
(ns native-lib-tutorial.main)

(cpp/raw "#include <compress.hpp>")

(defn -main [& args]
  (println "Hello, world!"))
```

When we try to run this project now, jank will fail to compile the code.

```bash
$ lein run
In file included from <<< inputs >>>:1:
input_line_1:4:10: fatal error: 'compress.hpp' file not found
    4 | #include <compress.hpp>
      |          ^~~~~~~~~~~~~~
error: Parsing failed.
─ internal/codegen-failure ─────────────────────────────────────────────────────────────────────────
error: Unable to compile C++ source.
```

This is where include directories come into play. Let's update our `project.clj`
to fix this issue!

```clojure
(defproject native-lib-tutorial "0.1-SNAPSHOT"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "0.2"]]
  :middleware [leiningen.jank/middleware]
  :main native-lib-tutorial.main

  ; Look here!
  :jank {:include-dirs ["native-lib"]}
  :profiles {:debug {:jank {:optimization-level 0}}
             :release {:jank {:optimization-level 2}}})
```

Now we can run the project again.

```bash
$ lein run
Hello, world!
```


So our jank code is including the native lib header, but we're not yet doing
anything with it. Let's call our actual `compress` function from jank now. Update the `-main`
function within `main.jank` to look like the following.

```clojure
(defn -main [& args]
  (if (empty? args)
    (println "Try passing some data to compress!")
    (let [input (first args)
          output (cpp/native_lib.compress input)]
      (println "input size" (count input) "output size" (count output)))))
```

Again, we're intentionally forgetting a step so we can see what happens. Let's
try to run this now!

```bash
$ lein run
Try passing some data to compress!

$ lein run "This is some data to compress!"
JIT session error: Symbols not found: [ _ZN10native_lib8compressERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE ]
error: Failed to materialize symbols: { (main, { _ZN3jtl6detail5panicINS_6resultIN4jank7runtime4orefINS4_3varEEENS_16immutable_stringEEEEEvRKT_, __orc_init_func.incr_module_3, DW.ref.__gxx_personality_v0, __clang_call_terminate, _ZZSt26__throw_bad_variant_accessjE9__reasons, _ZTSN19native_lib_tutorial4main7_main_1E, _ZNK19native_lib_tutorial4main7_main_115get_arity_flagsEv, _ZTIN19native_lib_tutorial4main7_main_1E, _ZN4jank7runtime8make_boxINS0_3obj17persistent_stringEJRA12_KcEQsr8behaviorE11object_likeIT_EEENS0_4orefIS7_EEDpOT0_, _ZNSt18bad_variant_accessD0Ev, $.incr_module_3.__inits.0, _ZTVSt18bad_variant_access, _ZSt26__throw_bad_variant_accessj, _ZNK3jtl6resultIN4jank7runtime4orefINS2_3varEEENS_16immutable_stringEE10expect_errEv, _ZN19native_lib_tutorial4main7_main_1C2Ev, _ZTSSt18bad_variant_access, _ZTISt18bad_variant_access, _ZN4jank7runtime8make_boxINS0_3obj17persistent_stringEJRA37_KcEQsr8behaviorE11object_likeIT_EEENS0_4orefIS7_EEDpOT0_, _ZN4jank7runtime8make_boxINS0_3obj17persistent_stringEJRA11_KcEQsr8behaviorE11object_likeIT_EEENS0_4orefIS7_EEDpOT0_, _ZN4jank7runtime7convertINSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEE11from_objectENS0_4orefINS0_6objectEEE, _ZNKSt18bad_variant_access4whatEv, _ZTVN19native_lib_tutorial4main7_main_1E, _ZN19native_lib_tutorial4main7_main_14callEN4jank7runtime4orefINS3_6objectEEE, _ZN4jank7runtime3obj12jit_functionD2Ev, _ZN4jank7runtime8make_boxERKN3jtl21immutable_string_viewE, _ZN19native_lib_tutorial4main7_main_1D0Ev, _ZNK4jank7runtime4orefINS0_3obj17persistent_stringEE5eraseEv }) }
─ internal/codegen-failure ─────────────────────────────────────────────────────────────────────────
error: Unable to compile C++ source.
```

Uh oh! There's a huge JIT (just in time) linker error. If we focus on the first
line, we can see `Symbols not found` and then this:

```cpp
_ZN10native_lib8compressERKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
```

This is a C++ mangled symbol, so if you plug it into something like
[demangler](https://www.demangler.com/), you can see that it demangles to:

```cpp
native_lib::compress(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>> const&)
```

That's our `compress` function!

All this means is that the C++ JIT runtime wasn't able to find a definition for
that symbol. The reason is that we didn't link our shared library in. Let's do
that in `project.clj` now. If we expand on the include directory we added
already, we can specify a library directory and the name of our library.

```clojure
  :jank {:include-dirs ["native-lib"]
         :library-dirs ["native-lib"]
         :linked-libraries ["compress"]}
```

With this change, we can now run our whole program.

```bash
$ lein run "This is some data to compress! Ideally, the output is smaller than the input."
input size 77 output size 74

$ lein run "Repeated strings are easier to compress. Repeated strings are easier to compress."
input size 81 output size 52
```

> [!NOTE]
> The way native linking works is that you link to `compress`, but the linker
> will actually look for `libcompress.a` and `libcompress.so` (or `libcompress.dylib`
> on macOS). Do not put the full file name in `:linked-libraries` unless
> you're also specifying the absolute path.

Finally, if we AOT (ahead of time) compile this project down to an executable,
we can run it without Leiningen.

```bash
$ lein compile
$ ./a.out
./a.out: error while loading shared libraries: libcompress.so: cannot open shared object file: No such file or directory
```

Oh no! Our `libcompress.so` isn't found, when we try to run our compiled
executable. If we inspect the binary with `ldd` (or `otool -L` on macOS), we can
see the linked libraries. Note how `libcompress.so` is `not found`.

```
$ ldd a.out
	linux-vdso.so.1 (0x00007fa6f0f64000)
	libm.so.6 => /usr/lib/libm.so.6 (0x00007fa6ebadd000)
	libLLVM.so.22.0git => /home/jeaye/projects/jank/compiler+runtime/build/llvm-install/usr/local/bin/../lib/libLLVM.so.22.0git (0x00007fa6e7000000)
	libclang-cpp.so.22.0git => /home/jeaye/projects/jank/compiler+runtime/build/llvm-install/usr/local/bin/../lib/libclang-cpp.so.22.0git (0x00007fa6e2c00000)
	libcrypto.so.3 => /usr/lib/libcrypto.so.3 (0x00007fa6e268c000)
	libz.so.1 => /usr/lib/libz.so.1 (0x00007fa6ebac4000)
	libzstd.so.1 => /usr/lib/libzstd.so.1 (0x00007fa6e25a7000)
	libcompress.so => not found
	libstdc++.so.6 => /usr/lib/libstdc++.so.6 (0x00007fa6e2313000)
	libgcc_s.so.1 => /usr/lib/libgcc_s.so.1 (0x00007fa6eba95000)
	libc.so.6 => /usr/lib/libc.so.6 (0x00007fa6e2101000)
	/lib64/ld-linux-x86-64.so.2 => /usr/lib64/ld-linux-x86-64.so.2 (0x00007fa6f0f66000)
	libedit.so.0 => /usr/lib/libedit.so.0 (0x00007fa6eba59000)
	libxml2.so.16 => /usr/lib/libxml2.so.16 (0x00007fa6e1fcc000)
	libncursesw.so.6 => /usr/lib/libncursesw.so.6 (0x00007fa6eb9ea000)
	libicuuc.so.78 => /usr/lib/libicuuc.so.78 (0x00007fa6e1dbe000)
	libicudata.so.78 => /usr/lib/libicudata.so.78 (0x00007fa6dfe29000)
```

Getting around this varies based on situation, but a quick workaround is to
tell the dynamic linker where else to look, when we run our program.

```bash
# Linux.
$ LD_LIBRARY_PATH=native-lib ./a.out "ABABABABABABABABABAB"
input size 20 output size 12

# macOS.
$ DYLD_LIBRARY_PATH=native-lib ./a.out "ABABABABABABABABABAB"
input size 20 output size 12
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
