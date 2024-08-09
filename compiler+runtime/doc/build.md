# Building jank
## Dependencies
For Debian-based distros, this should be all you need:

```bash
sudo apt-get install -y curl git git-lfs zip build-essential entr libssl-dev libdouble-conversion-dev pkg-config ninja-build python3-pip cmake debhelper devscripts gnupg zlib1g-dev entr libffi-dev clang libjemalloc-dev libreadline-dev
```

For Arch:

```bash
sudo pacman -S git git-lfs clang pkg-config cmake ninja make python3 libffi jemalloc entr
```

For macOS, try this:

```bash
brew install curl git git-lfs zip entr openssl double-conversion pkg-config ninja python cmake gnupg zlib jemalloc
```

Clone the repo as follows:

```bash
git clone --recurse-submodules https://github.com/jank-lang/jank.git

# If you didn't recurse submodules when cloning, you'll need to run this.
git submodule update --recursive --init
```

## Compiling Cling
Note that you must compile Cling/Clang/LLVM. This can take an hour or two,
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

At this point, you're ready to build jank.


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

## Packaging
There's also a script for installing jank and all its necessary dependencies.
Note that this includes a lot of header files, which are necessary for jank's
JIT compilation.

```bash
./bin/configure -GNinja -DCMAKE_BUILD_TYPE=Release
./bin/install
```
