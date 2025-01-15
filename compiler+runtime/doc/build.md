# Building jank
jank requires LLVM 19+. Make sure your package manager has it. If not, we have
scripts to compile your own. See the section on compiling Clang/LLVM below.

## Dependencies
For Debian-based distros, this should be all you need:

```bash
sudo apt-get install -y curl git git-lfs zip build-essential entr libssl-dev libdouble-conversion-dev pkg-config ninja-build cmake zlib1g-dev libffi-dev clang libclang-dev llvm llvm-dev libzip-dev libbz2-dev doctest-dev libboost-all-dev gcc g++ libgc-dev
```

For Arch:

```bash
sudo pacman -S git git-lfs clang llvm pkg-config cmake ninja make python3 libffi entr doctest boost libzip lbzip2 gc
```

For Nix:

Simply enter the provided development shell which provides all necessary dependencies before compiling jank itself.

```
# Run this in the jank repo after cloning.
nix develop ".#"
```

For macOS:

```bash
brew install curl git git-lfs zip entr openssl double-conversion pkg-config ninja python cmake gnupg zlib doctest boost libzip lbzip2 llvm@19 bdw-gc

# Ensure you have this set up in your shell.
export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)

# The output of installing LLVM via homebrew will also guide you to set this up:
export PATH="/opt/homebrew/opt/llvm/bin:${PATH}"
export LDFLAGS="-L/opt/homebrew/opt/llvm/lib ${LDFLAGS}"
export CPPFLAGS="-I/opt/homebrew/opt/llvm/include ${CPPFLAGS}"
```

Clone the repo as follows:

```bash
git clone --recurse-submodules https://github.com/jank-lang/jank.git

# If you didn't recurse submodules when cloning, you'll need to run this.
git submodule update --recursive --init
```

## Compiling jank
### Release
A typical release build just needs the following:

```bash
./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Release
./bin/compile
```

### Debug
To make a debug build, specify the build type when configuring.

```bash
./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Debug -Djank_tests=on
./bin/compile

# When developing, continuously run the tests locally.
./bin/watch ./bin/test
```

# Run jank
To run jank's repl, inside of /compiler+runtime, do
```bash
./build/jank repl
```

## Packaging
There's also a script for installing jank and all its necessary dependencies.
Note that this includes a lot of header files, which are necessary for jank's
JIT compilation.

```bash
./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Release
./bin/install
```

## Compiling Clang/LLVM
If you're making local changes to Clang/LLVM, you can build it along with jank
by using the following. This can take an hour or two,
depending on your machine. Building jank itself should take a minute or two.

```
cd compiler+runtime
mkdir -p build
cd build
export CC=clang; export CXX=clang++;
../bin/build-clang
cd -
export CC=$PWD/build/llvm-install/usr/local/bin/clang; export CXX=$PWD/build/llvm-install/usr/local/bin/clang++
```

Now configure and build jank as normal, but pass `-Djank_local_clang=on` when you configure.
