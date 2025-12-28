# Hello, Leiningen!
jank, on its own, is just a compiler and runtime. For non-trivial projects, you
will want a tool to manage your dependencies, profiles, resources, and
development workflows. For this, we use [Leiningen](https://leiningen.org/) (LINE-ing-en).
Leiningen is a project management tool for Clojure and jank is a dialect of
Clojure.

> [!NOTE]
> In the future, jank's recommended workflow will be to use the default Clojure CLI, but
> this tool is still being improved and it doesn't yet offer the excellent
> user experience that Leiningen does.

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
The recommended way to start a new project with jank is to use the jank template
within Leiningen. Let's create another hello world style program, but this time
using Leiningen to manage our project.

```bash
$ lein new org.jank-lang/jank hello_lein
```


> [!NOTE]
> If you use `lein new` without specifying `org.jank-lang/jank`, you will get a
> Clojure JVM project, not a jank project. Make sure you get a jank project.

## Running a Leiningen project
```bash
$ lein run
```
Hello, world!
