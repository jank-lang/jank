# The jank programming language [![codecov](https://codecov.io/gh/jank-lang/jank/branch/main/graph/badge.svg)](https://codecov.io/gh/jank-lang/jank) [![Sponsor](https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&link=https://github.com/sponsors/jeaye&color=red)](https://github.com/sponsors/jeaye)

jank is a Clojure dialect which aims to offer the best of both worlds: dynamic
and static. For both of these worlds, jank uses a single runtime, so the
transition from one to the other is not only seamless, it can be gradual.

https://jank-lang.org

For the current progress of jank and its usability, see the tables here: https://jank-lang.org/progress/

## Building locally
**NOTE:** jank is not very buildable right now. It's also not very usable right
now. See this issue for details: https://github.com/jank-lang/jank/issues/7

Clone the repo as follows:

```bash
$ git clone --recurse-submodules https://github.com/jank-lang/jank.git

# If you don't recurse submodules when deploying, you'll need to run this after
# cloning.
$ git submodule update --recursive --init
```

### Compiling
* `./bin/configure` -- For setting up the project.
* `./bin/compile` -- For one-off compilation.
* `./bin/test` -- For one-off testing.
* `./bin/watch-tests` -- For hot reloading tests on save.
* `./bin/install` -- For packaging.

### Release
A typical release build just needs the following:

```bash
$ ./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Release
$ ./bin/compile
```

### Debug
To make a debug build, specify the build type when configuring.

```bash
$ ./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Debug -Djank_tests=on
$ ./bin/compile
$ ./bin/watch-tests
```

### Packaging
There's also a script for installing jank and all its necessary dependencies.
Note that this includes a lot of header files, which are necessary for jank's
JIT compilation.

```bash
$ ./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Release
$ ./bin/install
```

### Nix
To aid in development, a `shell.nix` is provided. It's not necessary, but can be
useful for those with Nix who want a reliable way to get some development tools.
Just run `nix-shell` in the repo's root directory to get a shell with some
additional tools.
