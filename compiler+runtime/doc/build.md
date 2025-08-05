# Building jank
jank requires LLVM 21+ with changes not yet merged upstream. We have
a script to compile this from source. See the section on compiling Clang/LLVM below.

## Dependencies
For Debian-based distros, this should be all you need:

```bash
sudo apt-get install -y curl git git-lfs zip build-essential entr libssl-dev libdouble-conversion-dev pkg-config ninja-build cmake zlib1g-dev libffi-dev libzip-dev libbz2-dev doctest-dev gcc g++
```

For Arch:

```bash
sudo pacman -S gcc git git-lfs pkg-config cmake ninja make python3 libffi entr doctest libzip lbzip2 libxml2 libedit
```

For Nix:

Simply enter the provided development shell which provides all necessary dependencies before compiling jank itself.

```
# Run this in the jank repo after cloning.
nix develop ".#"
```

For macOS:

```bash
brew install curl git git-lfs zip entr openssl double-conversion pkg-config ninja python cmake gnupg zlib doctest libzip lbzip2

# Ensure you have this set up in your shell.
export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)

# The output of installing LLVM via homebrew will also guide you to set this up:
export PATH="/opt/homebrew/opt/llvm/bin:${PATH}"
export LDFLAGS="-L/opt/homebrew/opt/llvm/lib ${LDFLAGS}"
export CPPFLAGS="-I/opt/homebrew/opt/llvm/include ${CPPFLAGS}"
```

Clone the repo as follows:

```bash
git clone --depth 1 --single-branch --shallow-submodules --recurse-submodules https://github.com/jank-lang/jank.git

# If you didn't recurse submodules when cloning, you'll need to run this.
git submodule update --init --recursive --depth 1 --jobs 8
```

## Compiling Clang/LLVM
This can take an hour or two, depending on your machine. Building jank itself
should take a minute or two.

```
cd compiler+runtime
mkdir -p build
export CC=clang; export CXX=clang++;
./bin/build-clang
export CC=$PWD/build/llvm-install/usr/local/bin/clang; export CXX=$PWD/build/llvm-install/usr/local/bin/clang++
```

Now configure and build jank as normal, but pass `-Djank_local_clang=on` when you configure.

## Compiling jank
### Release
A typical release build just needs the following:

```bash
cd compiler+runtime
./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Release
./bin/compile
```

### Debug
To make a debug build, specify the build type when configuring.

```bash
cd compiler+runtime
./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Debug -Djank_test=on
./bin/compile

# When developing, continuously run the tests locally.
./bin/watch ./bin/test
```

# Run jank
To run jank's repl do
```bash
cd compiler+runtime
./build/jank repl
```

## Packaging
There's also a script for installing jank and all its necessary dependencies.
Note that this includes a lot of header files, which are necessary for jank's
JIT compilation.

```bash
cd compiler+runtime
./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Release
./bin/install
```
