# Guide: Packaging a source library
jank can package libraries which need building from source. There is no
limitation on the build system used, such as CMake, Automake, GNU Make, Scons,
etc. The jank build system can work with any build system, since we just invoke
it externally.

> [!NOTE]
> If you can [package a library using an installed system package](./packaging-system-lib.md) and
> `pkg-config`, prefer that. It's much simpler, more efficient, and more portable
> than building from source.

Before continuing, please read the guide on [packaging system libraries](./packaging-system-lib.md),
since it teaches the foundational knowledge of how packaging works with jank's
build system.

## Hello raylib
For this guide, we're going to package
[raylib](https://github.com/raysan5/raylib) from source. Although raylib has a
package in Nix and Homebrew, there is no Ubuntu/Debian package, which makes the
`pkg-config` approach far less portable. Instead, we'll build raylib from
source.

## A note on `-sys` packages
Even when we build from source, the pattern around `-sys` packages applies, so
be sure to read about that [here](./packaging-system-lib.md#a-note-on--sys-packages).

## Building raylib manually
To start with, let's make sure we can build raylib manually. Then we'll deal
with hooking that same flow into the jank build system. The first step in
building raylib is to just download it. For your package, it likely makes sense
to add raylib as a git submodule. You could also just clone it or download a
release from Github, as long as you have a `raylib-sys` directory for your
package with a `raylib` directory inside, which has the actual raylib
repository.

In our `raylib` directory, we'll make our own build directory, just like the
jank build system's [build directory](./build-cache.md).

```bash
❯ cd raylib

❯ mkdir build
mkdir: created directory 'build'

❯ cd build

❯ cmake .. -DBUILD_EXAMPLES=off -DBUILD_SHARED_LIBS=on
<CMake configure output>

❯ make
<Make output>

# On macOS, you'll see libraylib.dylib instead!
❯ ls raylib/libraylib.*
raylib/libraylib.so

❯ cd ../../

❯ rm -r raylib/build
```

That wasn't so bad! Now we just need to do the same from our `jank-build.bb`.

## Packaging raylib
Let's add our basic project:

```clojure
(defproject org.jank-lang.guide/raylib-sys "2026.07-1"
  :description "Raw package for raylib."
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :plugins [[org.jank-lang/lein-jank "2026.07-1"]]
  :middleware [leiningen.jank/middleware]
  :build-dependencies [[org.jank-lang.commons/jank-build-cmake "2026.06-6"]]
  :verbatim-paths ["raylib"])
```

Note the `jank-build-cmake` helper in the `:build-dependencies`. This is going
to do the heavy lifting for us. Also note the `:verbatim-paths`, which tells
Leiningen that we want the entire `raylib` directory to be included in our
`raylib-sys` package.

Now we just need our `jank-build.bb`. 

```clojure
;; Part 1.
(require '[babashka.fs :as fs]
         '[jank.build.cmake :as cmake])

;; Part 2.
(let [out-dir (:out-dir *input*)
      src-dir (fs/path (:src-dir *input*) "raylib")
      input   (assoc *input* :src-dir src-dir)]
  (cmake/build input {:defines {"BUILD_EXAMPLES" false}})

  ;; Part 3.
  (println (str "jank-build::include-dir=" (fs/path out-dir "include")))
  (println (str "jank-build::link-dir=" (fs/path out-dir "lib")))
  (println (str "jank-build::link-dir=" (fs/path out-dir "lib64")))
  (println (str "jank-build::link-library=" "raylib")))
```

We'll go over this part by part. Firstly, we require `babashka.fs` so we can do
some path manipulation. This namespace is available to all Babashka programs
without needing to add a dependency. Then we require `jank.build.cmake`, which
we added to our `:build-dependencies` above.

In part two, we invoke the `jank.build.cmake` helper, but we change the source
directory to be the nested `raylib` directory. The CMake helper expects the
`CMakeLists.txt` file to be within the source directory. The CMake helper will
automatically handle building the project in the build directory and installing
the final artifacts to the output directory.

In part three, we just print the necessary directives to tell the jank build
system where to find headers, libraries, and which libraries to link.

That's it! Now, if we wanted to use this package locally, we could install it
with `lein install` and then add it as a dependency to another project.

```bash
❯ lein install
Created /home/jeaye/projects/raylib-sys/target/raylib-sys-2026.07-1.jar
Wrote /home/jeaye/projects/raylib-sys/pom.xml
Installed jar and pom into local repo.
```

## Using the new package
Let's create a new project and use our `raylib-sys` library.

```bash
❯ lein new org.jank-lang/jank hello-raylib
Generating a project called hello-raylib based on the 'jank' template.
```

Then we need to add our dependency to the `project.clj`:

```clojure
  :dependencies [[org.jank-lang.guide/raylib-sys "2026.07-1"]]
```

Before we add any raylib code, let's try to run our project and make sure
everything builds correctly.

```bash
❯ lein run
Extracting [org.jank-lang.guide/raylib-sys 2026.07-1]
 Compiling [org.jank-lang.guide/raylib-sys 2026.07-1]
Hello, world!
```

Nice! Let's take a look at the generated output directory, to see what's inside.
Note that your directory name may be different, due to the hashing, but it'll be
in the same place:

```bash
❯ ls target/debug/_cache/raylib-sys-2026.07-1-out-cv799vKey7U76tllczT0Hw/
include  jank-build-cache.txt  jank-build-fingerprint.txt  lib64
```

Our headers will be in the `include` directory and, in this case, our libraries
will be in `lib64`. These may end up being named different things on your
machine, too. That's the flexibility of the jank build system: all of this is
handled by CMake and then the jank build system just connects the dots.

Speaking of which, let's wrap this up by writing some raylib code!

```clojure
(ns hello-raylib.main
  (:include "raylib.h"))

(defn -main [& args]
  (cpp/InitWindow 200 100 "raylib demo")
  (cpp/SetTargetFPS 60)

  (while (cpp/! (cpp/WindowShouldClose))
    (let [time (cpp/GetTime)
          color (cpp/ColorFromHSV (mod (* 100.0 time) 360.0) 0.5 0.5)]
      (cpp/BeginDrawing)
      (cpp/ClearBackground cpp/RAYWHITE)
      (cpp/DrawText "Hello jank!" 50 30 20 color)
      (cpp/EndDrawing)))

  (cpp/CloseWindow))
```

Now you should be able to run this and see a raylib window rendering some
colored text.

```bash
❯ lein run
INFO: Initializing raylib 6.1-dev
INFO: Platform backend: DESKTOP (GLFW)
<bunch of other raylib output>
```

## Summary
Packaging a source library for jank involves answering the same four key
questions as with system libraries. On top of that, we just need to utilize our
source directory, build directory, and out directory with the project's build
system in order to put the files where jank can use them. Take a look at the
CMake build script helper
[here](https://github.com/jank-lang/commons/blob/main/jank-build-cmake/src/jank/build/cmake.bb)
for a peek behind the scenes of what we used for this guide.

Also, take a look at the official
[raylib-sys](https://github.com/jank-lang/commons/tree/main/raylib-sys) jank
commons package, since it looks just like the one we made here!
