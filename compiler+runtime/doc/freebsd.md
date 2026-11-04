# jank on FreeBSD

jank builds and runs on **FreeBSD 14.4 (amd64)** with the FreeBSD `llvm22`
package. LLVM 23 is accepted by the build, but only 22 has been verified.

## Quick start: test from Linux with a container
FreeBSD can't run in a Linux container, so the container test boots the official
FreeBSD cloud image under QEMU (using KVM when `/dev/kvm` exists), copies your
checkout into it, provisions it, then builds, tests and packages jank. CI runs exactly
this.

```sh
compiler+runtime/bin/freebsd-container-test --build-only   # compile only
compiler+runtime/bin/freebsd-container-test                # full: tests + pkg smoke
compiler+runtime/bin/freebsd-container-test --keep-vm      # leave the VM up to debug
```

It needs Docker (or Podman) and your submodules checked out. Logs and the `.pkg` end
up in `freebsd-out/`. The FreeBSD image is cached in the `jank-freebsd-vm-cache`
volume, or in `--cache DIR`. Without KVM, the VM is emulated and very slow.

## Building on a FreeBSD host
Install the prerequisites as root once:

```sh
compiler+runtime/bin/freebsd-provision      # or: --build-only
```

This installs `shells/bash cmake ninja pkgconf git python3 llvm22 boost-libs libzip
libffi`, plus `babashka clojure leiningen openjdk25` for the test suite. FreeBSD's
`babashka` package is the Linux binary, so the script also enables the Linuxulator.
zlib is part of the base system.

Then, as any user:

```sh
cd compiler+runtime
bin/freebsd-build --preflight   # check prerequisites
bin/freebsd-build               # build, then run jank-test and the bash e2e suite
bin/freebsd-build --build-only  # build only
bin/freebsd-build --install     # install to --prefix after the tests pass
bin/freebsd-build -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON   # extra CMake args
```

Defaults: build dir `compiler+runtime/build-freebsd`, which is reused, never wiped;
LLVM from `/usr/local/llvm22`; jobs from `sysctl hw.ncpu`, capped by `--max-jobs`
(default 8). C++ compilation needs roughly 2 GiB per job. See `--help` for every
option.

### Submodule patches
cpptrace and CppInterOp need small FreeBSD patches, kept in `patches/freebsd/`.
`freebsd-build` applies them with `bin/apply-freebsd-patches` before configuring.
That script requires each submodule to be at the commit pinned in `base.txt`, skips
patches that are already applied, and checks every patch before writing anything.
Use `--check` for a dry run.

## Packaging
`bin/freebsd-package` turns a finished build into a native `pkg(8)` archive without
root:

```sh
bin/freebsd-package -B build-freebsd -o pkg-out
```

It runs `cmake --install` into a private `DESTDIR`, writes a `+MANIFEST` and plist,
and calls `pkg create`. The runtime dependencies (`llvm22`, and `boost-libs`, whose
headers JIT/AOT compilation uses) get their version and origin from the installed pkg
database. A missing dependency is an error. The script refuses to reuse a non-empty
staging or output directory.

`test/freebsd/smoke BUILD_DIR OUT_DIR` packages a build, runs both the staged binary
and a `pkg add`-installed one, and runs `jank check-health`. It installs into the
system, so only run it on a disposable host.

## Platform notes
- libcxxrt compares exception RTTI by address, so jank binds the host's RTTI symbols
  into the JIT. Otherwise, exceptions thrown on one side wouldn't be caught on the
  other.
- `--perf` is rejected, since LLVM's perf support crashes on FreeBSD (it relies on Linux's perf subsystem).
- `pthread_tryjoin_np` doesn't exist, so as on macOS, `future-cancelled?` can't report
  cancellation of a running future.
- The executable path comes from `sysctl(KERN_PROC_PATHNAME)`, since procfs isn't
  mounted by default.

## Tests for the tooling itself
These run on any POSIX host:

```sh
compiler+runtime/test/freebsd-tooling/run      # build/package scripts, with mocks
compiler+runtime/test/freebsd-patches/run.sh   # the patch applier, with fixtures
```
