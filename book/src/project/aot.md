# AOT compiling projects
Given a jank Leiningen project, you can ahead of time (AOT) compile all of your
code and dependencies to an executable using this command.

```bash
lein compile
```

As with GCC, Clang, etc, the default output name is `a.out`. When you invoke
your executable, your `-main` function will be called.

> [!NOTE]
> There is not yet a way to change the output name using Leiningen, but this
> will be implemented.

## Building for release
By default, `lein compile` will build you a debug executable with fewer
optimizations enabled. To get a release executable, you can enable the `release`
profile.

```bash
lein with-profile release compile
```

## Distributing your executables
Currently, AOT compiled executables still depend on jank, Clang, and LLVM. This
makes distribution more complicated, especially since the Clang/LLVM version
jank requires is bleeding edge. We are working on ways to both better package
these together as well as to provide options for building executables which are
standalone.
