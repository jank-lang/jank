# The jank programming language [![Build Status](https://app.travis-ci.com/jank-lang/jank.svg?branch=main)](https://travis-ci.com/github/jank-lang/jank) [![codecov](https://codecov.io/gh/jank-lang/jank/branch/main/graph/badge.svg)](https://codecov.io/gh/jank-lang/jank) [![Sponsor](https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&link=https://github.com/sponsors/jeaye&color=red)](https://github.com/sponsors/jeaye)

jank is a Clojure dialect which aims to offer the best of both worlds: dynamic
and static. For both of these worlds, jank uses a single runtime, so the
transition from one to the other is not only seamless, it can be gradual.

https://jank-lang.org

## Building locally
**NOTE:** jank is not very buildable right now. It's also not very usable right
now. See this issue for details: https://github.com/jank-lang/jank/issues/7

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
[LICENSE](https://github.com/jank-lang/jank/blob/main/LICENSE) file.
