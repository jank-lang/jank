# Guide: Packaging a system library
Packaging a system library is generally very straight-forward. We need to
identify these things:

1. Does this library need any preprocessor defines? Generally, the answer is no.
2. Where are the include headers stored?
3. Where are the libraries stored?
4. What are the libraries named?

That's it! Fortunately, `pkg-config` handles this for most system libraries.
Before we write a build script, let's first answer all four questions needed to
package sqlite3. If we invoke `pkg-config` and ask for both C flags and libs,
we'll actually get an answer to every single question above.

```bash
❯ pkg-config --cflags --libs sqlite3
-I/nix/store/vyd6g9viqafhzr97dq8zsbksdf4w5avm-sqlite-3.51.2-dev/include -L/nix/store/yg1gv8db04ldrnmdhykq8zjqqg6pg5kd-sqlite-3.51.2/lib -lsqlite3
```

> [!NOTE]
> If you see an error like `Package 'sqlite3' not found`, make sure that
> you have sqlite3 installed.

Note that this is the output for my particular NixOS system. On a different
distro, like Arch, the output may simply be `-lsqlite3` and nothing else. This is an
important thing to note about `pkg-config`. You cannot just invoke it on your
machine and then hard-code the results in your build script. You have to
actually invoke `pkg-config` in the build script, so that it can fetch the
correct flags for the user's machine.

## A note on `-sys` packages
When we wrap system packages, we end up creating a special `-sys` package. The
goal of `-sys` packages is just to make the system library available, without
providing any higher level Clojure-style wrapper for the API. This allows `-sys`
packages to be reused by different higher level abstraction libraries and it
separates the concerns of the libraries. In our example here, for sqlite3, we're
writing the `sqlite3-sys` package.

If you're creating a `-sys` package, please consider adding it to the [jank commons](https://github.com/jank-lang/commons).

## Writing a small package
Our `sqlite3-sys` package only needs two things:

1. A `project.clj`, defining how the package will be named and versioned
2. A `jank-build.bb`, defining how the package will be "built"

For us, "building" just means running `pkg-config` to answer our four questions.
Here's an example `project.clj`:

```clojure
(defproject org.jank-lang.guide/sqlite3-sys "2026.07-1"
  :description "Raw package for sqlite3."
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :plugins [[org.jank-lang/lein-jank "2026.07-1"]]
  :middleware [leiningen.jank/middleware]
  :build-dependencies [[org.jank-lang.commons/jank-build-pkg-config "2026.06-1"]])
```

Note that we add `:build-dependencies` so that we can grab
`jank-build-pkg-config`. This is a helper for the `jank-build.bb` we're going to
write. It's going to call `pkg-config` for us and then turn the output of that
into jank build system directives. Here's our `jank-build.bb`:

```clojure
(require '[jank.build.pkg-config :refer [pkg-config]])

(pkg-config "sqlite3")
```

That's it! Now, if we wanted to use this package locally, we could install it
with `lein install` and then add it as a dependency to another project.

```bash
❯ lein install
Created /home/jeaye/projects/sqlite3-sys/target/sqlite3-sys-2026.07-1.jar
Wrote /home/jeaye/projects/sqlite3-sys/pom.xml
Installed jar and pom into local repo.
```

## Using the new package
Let's create a new project and use our `sqlite3-sys` library.

```bash
❯ lein new org.jank-lang/jank hello-sqlite3
Generating a project called hello-sqlite3 based on the 'jank' template.
```

Then we need to add our dependency to the `project.clj`:

```clojure
  :dependencies [[org.jank-lang.guide/sqlite3-sys "2026.07-1"]]
```

And we need to call sqlite3 from our jank code:

```clojure
(ns hello-sqlite3.main
  (:include "sqlite3.h"))

(defn -main [& args]
  (println (cpp/sqlite3_libversion)))
```

Now, when we try to run our program, the jank build system will find the
`sqlite3-sys` package, extract it, run the `jank-build.bb` script, and propagate
the `pkg-config` flags up to our jank invocation.

```bash
❯ lein run
Extracting [org.jank-lang.guide/sqlite3-sys 2026.07-1]
 Compiling [org.jank-lang.guide/sqlite3-sys 2026.07-1]
3.51.2

# Future runs won't need to extract/build anything.
❯ lein run
3.51.2
```

Note that your sqlite3 version may be different from mine here. That's ok.

> [!NOTE]
> If you see an error like `Failed to find library 'sqlite3'`, make sure that
> you have sqlite3 installed. If you're on macOS, make sure that your
> `PKG_CONFIG_PATH` can find the homebrew config for sqlite3.
>
> ```bash
> export PKG_CONFIG_PATH="/opt/homebrew/opt/sqlite/lib/pkgconfig:$PKG_CONFIG_PATH"
> ```

## Summary
Packaging a system library for jank involves answering the four key questions.
Tools like `pkg-config` can do a lot of this for us! Some packages don't have
`pkg-config` entries, which can make this work more manual. Other packages have
their own version of `pkg-config`, like `curl-config`, which accept similar
flags. Take a look at the `pkg-config` build script helper
[here](https://github.com/jank-lang/commons/blob/main/jank-build-pkg-config/src/jank/build/pkg_config.bb)
for a peek behind the scenes of what we used for this guide.

Also, take a look at the official
[sqlite3-sys](https://github.com/jank-lang/commons/tree/main/sqlite3-sys) jank
commons package, since it looks just like the one we made here!
