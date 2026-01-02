# How to get a stack trace
If jank is crashing, or your AOT compiled program is, you may be asked to
provide a stack trace. In case you're not familiar with how to do this, here's a
quick rundown for both Linux (gdb) and macOS (lldb).

> [!NOTE]
> Getting a stack trace requires invoking a debugger with your jank command.
> If you're using Leiningen to invoke jank, you can get the underlying command
> by passing `-v` to Leiningen. For example, `lein run -v`.

Let's say we're trying to run `jank run foo.jank`.

## Linux
Make sure you have gdb installed. This is likely already installed, but if
it's not, it is definitely in your package manager's repos and it's likely just
called `gdb`. Once you have gdb, you can use the following.

```bash
$ gdb --args jank run foo.jank
> run
# Do whatever is needed to cause the crash.
> backtrace
# Copy this to share with others.
> quit
```

> [!NOTE]
> If you want to break when an exception is thrown, use the `catch throw`
> command in gdb before you `run`.

## macOS
On macOS, you should have lldb installed as part of your developer tools.
However, you can get newer versions from Homebrew as part of the `llvm` package.

```bash
$ lldb -- jank run foo.jank
> run
# Do whatever is needed to cause the crash.
> backtrace
# Copy this to share with others.
> quit
```

> [!NOTE]
> If you want to break when an exception is thrown, use the
> `breakpoint set -E c++` command in lldb before you `run`.
