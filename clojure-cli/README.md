# Clojure CLI with Jank

Clojure CLI can be used as a build tool for jank projects.

## Setup

Clojure CLI will mostly be used to calculate the `--module-path` for the `jank` command.

Put jank source files in the `src` directory like a normal Clojure project and
add it to the `:path`. You can pull git, maven, and local dependencies like a
normal Clojure project.

Jank tests go in the `test` directory, and we will add that to the `:paths`
using the `:test` alias activated with `-A:test`.

We will use the linked [test project](test-project) as an example.
Here is the `deps.edn` for that project.

```clojure
{:deps {jank-clojure-cli-test/subproject {:local/root "subproject"}}
 :paths ["src"]
 :aliases
 {:delete-clojure {;; an empty project 
                   org.clojure/clojure {:local/root "delete-clojure-subproject"}}
  :test {:extra-paths ["test"]}}}
```

The following assumes we are in the `test-project` directory

## Starting a dev REPL

```clojure
jank --module-path $(clojure -A:test -Spath) repl
```

## Run a file

```clojure
jank --module-path $(clojure -Spath) run-main jank-cli-test.core 1
```

## (Optional) Removing Clojure

A caveat is that the `org.clojure/clojure` dependency cannot be removed without a workaround.
This is only problematic if there is a classpath clash between jank and clojure or its dependencies
(e.g., both define the same `.cljc` file).

So far this does not seem to be the case, however, here's how it's done if needed.

Create a local project with the following `deps.edn`:

```clojure
mkdir delete-clojure-subproject
cd delete-clojure-subproject
echo '{:paths []}' > deps.edn
```

Add the following entry to your root deps.edn.
```clojure
 :aliases
 {:delete-clojure {;; an empty project 
                   org.clojure/clojure {:local/root "delete-clojure-subproject"}}}
```

Then add the `delete-clojure` alias when calculating `--module-path`.

```shell
jank --module-path $(clojure -A:test:delete-clojure -Spath) repl
```
