# Installing jank
If you're on any of the below systems, you can install jank using your system's
package manager.

## Homebrew (macOS or Linux)
```sh
brew install jank-lang/jank/jank
```

If you can an error about `git-lfs`, try the following:

```sh
git lfs install
sudo ln -s "$(which git-lfs)" "$(git --exec-path)/git-lfs"
```

## Distrobox (Linux)

```
git clone --recurse-submodules https://github.com/jank-lang/jank.git
cd jank/compiler+runtime/
curl -s https://raw.githubusercontent.com/89luca89/distrobox/main/install | sudo sh
distrobox create jank-ubuntu --image ubuntu:24.10
distrobox enter jank-ubuntu
sudo apt-get install -y curl git git-lfs zip build-essential entr libssl-dev libdouble-conversion-dev pkg-config ninja-build cmake zlib1g-dev libffi-dev clang libclang-dev llvm llvm-dev libzip-dev libbz2-dev doctest-dev gcc g++ libgc-dev
export CC=clang; export CXX=clang++
./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Debug
./bin/compile
```

Start ( `jank/compiler+runtime/`:

```
build/jank repl
```
N.b. it will continue to work even if you [remove](https://distrobox.it/#uninstallation) distrobox or the `jank-ubuntu` image: `distrobox stop jank-ubuntu;distrobox rm jank-ubuntu`

## Anything else
If nothing above matches what you have, you can still build jank by following
the docs [here](./build.md).
