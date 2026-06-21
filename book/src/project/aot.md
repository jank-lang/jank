# AOT compiling
Given a jank Leiningen project, you can ahead of time (AOT) compile all of your
code and dependencies to an executable using this command.

```bash
lein compile
```

Your executable will be named based on your current project and can be found
within `target/debug/<project name>` by default. When you invoke your
executable, your `-main` function will be called.

## Building for release
By default, `lein compile` will build you a debug executable with fewer
optimizations enabled. To get a release executable, you can enable the `release`
profile.

```bash
lein with-profile +release compile
```

## AOT runtime selection
By default, AOT compilation will target jank's `static` runtime. This means that
the compiled binary will not link to Clang/LLVM and all of its functionality
will be baked in. Since Clang/LLVM is not linked in, JIT compilation is not
possible at runtime. Calling something like `eval` will throw. This is very
similar to a Graal native image.

If you need to be able to JIT compile code from your AOT compiled binary, you'll
need to enable the `dynamic` runtime. From jank's command line, you can use
`--runtime dynamic`, but you can also just set this in your Leiningen project.

```clojure
:profiles {:release {:jank {:runtime :dynamic}}}
```
