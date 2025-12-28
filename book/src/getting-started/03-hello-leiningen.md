# Hello, Leiningen!
jank, on its own, is just a compiler and runtime. For non-trivial projects, you
will want a tool to manage your dependencies, profiles, resources, and
development workflows. For this, we use [Leiningen](https://leiningen.org/) (LINE-ing-en).
Leiningen is a project management tool for Clojure and jank is a dialect of
Clojure.

> [!NOTE]
> In the future, jank's recommended workflow will be to use the default Clojure CLI tool, but
> it's still being improved and it doesn't yet offer the excellent
> user experience that Leiningen does. For now, it is not recommended for jank
> projects.

## Installing Leiningen
Leiningen is available in basically every package manager as `leiningen`.

```bash
# If you're on macOS.
$ brew install leiningen

# If you're on Ubuntu (or similar).
$ sudo apt install -y leiningen

# If you're on Arch (or similar).
$ yay -S leiningen
```

For more details, see the [Leiningen docs](https://leiningen.org/#install).

## Creating a project with Leiningen
Now that Leiningen is installed, we can use it to create a new jank project.
Let's create another hello world style program.

```bash
$ mkdir ~/projects
$ cd ~/projects
$ lein new org.jank-lang/jank hello_lein
$ cd hello_lein
```


> [!NOTE]
> If you use `lein new` without specifying `org.jank-lang/jank`, you will get a
> Clojure JVM project, not a jank project. Make sure you get a jank project.

## The layout of a Leiningen project
Inside the `hello_lein` directory, you will find some files have already been
created.

```bash
$ ls
LICENSE  project.clj  src  test
```

Most importantly, the `project.clj` is the file which controls Leiningen and
stores all meta information about your project. To start with, our `project.clj`
will look similar to this:

```clojure
(defproject hello_lein "0.1-SNAPSHOT"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "0.2"]]
  :middleware [leiningen.jank/middleware]
  :main hello-lein.main
  :profiles {:debug {:jank {:optimization-level 0}}
             :release {:jank {:optimization-level 2}}})
```

Your versions may differ, but the overall structure will remain. Our
`project.clj` defines some useful aspects to Leiningen.

1. The project name `hello_lein` and version `0.1-SNAPSHOT`
2. `:license`: The license of our project, which defaults to MPL since jank uses
   it. You are free to change this.
3. `:dependencies`: Our project dependencies. More on this in another chapter.
4. `:plugins`: The `lein-jank` plugin, and its `:middleware`, which is used to configure our
   project for jank instead of Clojure JVM.
5. `:main`: The entrypoint of our program, which contains our `-main` function.
6. `:profiles`, which allows us to enable different flags and build modes.

Inside `src/hello_lein/main.jank`, we will see the code for our project.

```clojure
(ns hello-lein.main)

(defn -main [& args]
  (println "Hello, world!"))
```

## Running a Leiningen project
Running your project involves starting at our `:main` file, loading all required
files, and then calling the `-main` function. Leiningen will help us with
setting everything up so that jank can do this.

```bash
$ lein run
Hello, world!
```

## Testing a Leiningen project
Leiningen has support for easily running all tests for a project. Tests are
written in the `test/` directory. The jank template provided us with an example
test which will fail.

```bash
$ lein test
```

> [!NOTE]
> This is not yet supported in jank. Accomplishing this requires finding
> all test namespaces and running them using `clojure.test`.

## Compiling a Leiningen project
It's possible to AOT (ahead of time) compile our whole Leiningen project to an
executable. This involves compiling every one of our source files and
dependencies and then linking them all together. Leiningen makes this easy.

```bash
$ lein compile
$ ./a.out
Hello, world!
```

As with GCC, Clang, etc, the default output name is `a.out`. When we invoke
that, we see our printed hello world.

> [!NOTE]
> There is not yet a way to change the output name using Leiningen, but this
> will be implemented.
