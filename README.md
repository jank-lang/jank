# The jank programming language [![Build Status](https://app.travis-ci.com/jeaye/jank.svg?branch=main)](https://travis-ci.org/jeaye/jank) [![codecov](https://codecov.io/gh/jeaye/jank/branch/main/graph/badge.svg)](https://codecov.io/gh/jeaye/jank)

jank is a Clojure dialect which aims to offer the best of both worlds: dynamic
and static. For both of these worlds, jank uses a single runtime, so the
transition from one to the other is not only seamless, it can be gradual.

https://jank-lang.org

## Building locally
Use [Nix](https://nixos.org/manual/nix/stable/). After cloning, use `nix-shell`
to enter an environment with all necessary deps. The Nix shell will also set up
some bash functions for you.

* `jank-configure` -- For setting up the project.
* `jank-compile` -- For one-off compilation.
* `jank-test` -- For one-off testing.
* `jank-watch-tests` -- For test hot reloading on save.

### Release
A typical release build just needs the following:

```bash
$ jank-configure
$ jank-compile
```

### Debug
To make a debug build, specify the build type when configuring.

```bash
$ jank-configure -Djank_build_type=debug
$ jank-compile
```

## License
jank is under a strict copyleft license; see the
[LICENSE](https://github.com/jeaye/jank/blob/main/LICENSE) file.
