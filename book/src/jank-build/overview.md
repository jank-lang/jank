# Build system overview
jank provides a custom native build system with the goal of making it easy to
build your jank programs on top of C and C++ libraries.
[Cargo](https://doc.rust-lang.org/cargo/) is a big inspiration for the design of
jank's build system.

Some dependencies are installed by the system and others need to be built from
source. jank's build system handles both of these cases. It's important to note
that jank's build system doesn't aim to replace existing native build systems
like CMake, but it does integrate with them via custom build scripts.

The foundational aspect of jank's build system is the `jank-build.bb` script.
Each jank package may have one `jank-build.bb` script and its presence indicates
to the jank build system that there's work to be done to build that package.

The scripts itself is executed with Babashka and may use any Babashka APIs
available. The jank build system provides some particular APIs to aid in finding
system packages and building projects from source.

Each script has two jobs, which we'll explain separately.

1. Print directives to `stdout` which tell the jank build system to add flags
   to its jank invocation
2. Find or build files and install them in the provided `out` directory

By default, each build script for a dependency will run whenever any of the
build flags change. For example, if you change your optimization level, all
dependencies will be rebuilt. However, build scripts can also print directives
which tell the jank build system to that package when other things change.

## Directives
Each printed directive is a single line printed to `stdout` by the
`jank-build.bb` script. Any lines printed which don't start with `jank-build::`
are ignored by the build system. Every directive can be provided any number of
times and will always build on the previously printed directives.
The directives fit into two categories:

1. Build flags
2. Re-run conditions

### Build flags
For the build flags, you can specify preprocessor defines, include directories
which contain header files, link directories which contain libraries, and the
library names to actually link.

#### `jank-build::define=FOO`
Adds the `-DFOO` flag to the jank invocation. This can also be used with a value,
with `jank-build::define=FOO=1`, which then adds the `-DFOO=1` flag.

#### `jank-build::include-dir=path`
Adds the `-I path` flag to the jank invocation. The path should be absolute.

#### `jank-build::link-dir=path`
Adds the `-L path` flag to the jank invocation. The path should be absolute.

#### `jank-build::link-library=lib`
Adds the `-l lib` flag to the jank invocation. Library names, relative paths,
file names, and absolute paths are supported.

### Re-run conditions
Re-run condition directives tell the jank build system when to re-run the build
script. By default, it will happen whenever any files within the source
directory of the package change.

#### `jank-build::rerun-if-changed=path`
Informs the jank build system to re-run this build script if a particular file
changes. The `path` is expected to be relative to the source directory of the
package. If the `path` is a directory, all files within that directory will be
watched.

Note: If no `rerun-if-changed` directive is provided, all files within the source
directory will be watched.

#### `jank-build::rerun-if-env-changed=FOO`
Informs the jank build system to re-run this build script if the environment
variable `FOO` changes. This is mainly expected to be used for variables like
`CC` and `CXX`, but anything can work.

Note: If no `rerun-if-env-changed` directive is provided, no environment
variables will be watched.

## Finding or building things
The second thing build scripts can do is prepare files for jank consumption.
This may involve building projects with CMake or similar. In order to do this
effectively, the jank build system has the concept of three key directories per
build script invocation:

1. Source directory (`:src-dir`) -- This is where the package's source files
   are.
2. Build directory (`:build-dir`) -- This is a temporary directory where build files can be
   placed.
3. Out directory (`:out-dir`) -- This is where the final artifacts must be installed in order
   to be used by jank.

On top of that, build scripts are provided with the following:

1. A map of build inputs (`:inputs`) which maps from package name to out
   directory -- This is useful when your package depends on the output of
   another package.
2. The optimization level to use (`:optimization-level`) -- This is an integer
   from `0` to `3` inclusive. `0` means no optimizations.
3. Whether or not to build a static lib (`:static?`).

The inputs to a build script are available via `*input*`. Here's an example:

```clojure
{:src-dir "/path/to/project"
 :build-dir "/tmp/jank-build-6616793965093062497"
 :out-dir "/path/to/project/target/debug/_cache/imgui+glfw-out-XXX"
 :inputs {"org.jank-lang.commons/imgui-sys" "/path/to/project/target/debug/_cache/imgui-sys-2026.06-6-out-HsZD6cjvMzkPAeaJZZQAKw"
          "org.jank-lang.commons/glfw-sys" "/path/to/project/target/debug/_cache/glfw-sys-2026.06-1-out-zCGlvgSQ65jN8ZQwixkVIA"
          "org.jank-lang.commons/imgui-glfw-sys" "/path/to/project/target/debug/_cache/imgui-glfw-sys-2026.06-6-out-DY2lSndonGjZEkhw7JE1TQ"
          "org.jank-lang.commons/gl-sys" "/path/to/project/target/debug/_cache/gl-sys-2026.06-1-out-qCuPOQm5Ch-yJlIvZOBPqA"
          "org.jank-lang.commons/imgui-opengl2-sys" "/path/to/project/target/debug/_cache/imgui-opengl2-sys-2026.06-6-out-OisnnGXhsy1cebbvy8iM9A"}
 :optimization-level 0
 :static? true}
```


### Sandboxing
By default, build scripts run in a sandbox which only has write access to the
build and out directories. In-source builds will not work. Read access is given
to system-level directories, but not `/home`. To disable the sandbox, you can
run Leiningen with `--disable-sandbox`. For example: `lein run --disable-sandbox`.

## Build script dependencies
Your build scripts may have dependencies of their own, which are not available
to your jank application. This is handled by adding `:build-dependencies` to
your `project.clj`. For example:

```clojure
:build-dependencies [[org.jank-lang.commons/jank-build-cmake "2026.06-6"]]
```

## Top-level build scripts
Your jank project may also have a `jank-build.bb` of its own, stored in the
top-level directory of your project, adjacent to your `project.clj`. By default,
this build script always runs, unless you provide any `rerun-if-changed`
directives in it. If you just want to add some `-D`, `-I`, `-L`, or `-l` flags
to your project, this is where you should do it.

## Troubleshooting build failures
If a build script fails, the jank build system will automatically print the
`stdout` and `stderr` from the build. You can also opt into verbose mode by
providing `-v` to your Leiningen invocation. For example: `lein compile -v`.
