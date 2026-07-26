# The build cache
jank and the jank build system store the output of builds into the "target" and
"build" directories. By default, the target directory is `target` and the build
directory is `target/_cache`, both relative to the `project.clj` or current
directory of the `jank` execution. Within Leiningen projects, the jank
template automatically sets up target directories per-profile. This changes the
`debug` profile target directory to `target/debug`, for example. To change the
target directory with `jank` directly you can use the `--target-dir` flag.
Similarly, the `--build-dir` flag can be used, but it's not currently
possible to change the build directory within `project.clj`.

There are two types of artifacts:

* Final build artifacts
  * These files are meant for your consumption and are placed within the target
    directory.
* Intermediate build artifacts
  * These are internal files to the jank build system and are generally not
    consumed directly. They are stored within the build directory.

## Example trees
### Leiningen project
For a Leiningen project, `json-query`, an example tree with a debug build looks like this:

```
target/
├── debug
│   ├── _cache
│   │   └── json-query
│   │       ├── x86_64-unknown-linux-gnu-211eea4be7942af843e902b894e93b6cc665b2aa6943f36f5bca4ddde080972b
│   │       │   └── json_query
│   │       │       └── main.o
│   │       └── x86_64-unknown-linux-gnu-5e3948b46dee3bb4f17fc699e821182481fb5465155bdbc359678f7e6a8db9da
│   │           └── json_query
│   │               └── main.o
│   └── json-query
└── stale
    └── leiningen.core.classpath.extract-native-dependencies
```

Note that there are two builds of `json-query`, with two different hashes. This
happens when your compiler flags change.

Also note that the final executable is at `target/debug/json-query`.

### Direct jank invocation
If you invoke jank directly to compile code, you'll still have a target and
build directory created for you. Here's an example of compiling a local
`hello.jank`.

```
.
├── hello.jank
└── target
    ├── a.out
    └── _cache
        └── x86_64-unknown-linux-gnu-deb515fb6fcde162c8cefee396e3090f348d5ec8e2d1a863abf325fa79682321
            └── hello.o
```
