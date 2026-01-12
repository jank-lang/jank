# Checking jank's health
Once jank is installed, you can query its health at any time. Here's an example
output of jank installed via Homebrew on macOS.

```
$ jank check-health
─ system ───────────────────────────────────────────────────────────────────────────────────────────
─ ✅ operating system: macos
─ ✅ default triple: arm64-apple-darwin25.0.0

─ jank install ─────────────────────────────────────────────────────────────────────────────────────
─ ✅ jank version: jank-0.1-768f8310ce0f3d61b01f2df0f0e66ab9c9df1984
─ ✅ jank resource dir: ../lib/jank/0.1
─ ✅ jank resolved resource dir: /opt/homebrew/Cellar/jank/0.1/bin/../lib/jank/0.1 (found)
─ ✅ jank user cache dir: /Users/jeaye/.cache/jank/arm64-apple-darwin25.0.0-f33ec85999b436c281e9fba631425b57189670f96ba3166f2d327cd1543b516d (found)

─ clang install ────────────────────────────────────────────────────────────────────────────────────
─ ⚠️ configured clang path: /Users/runner/work/jank/jank/compiler+runtime/build/llvm-install/usr/local/bin/clang++ (not found)
─ ✅ runtime clang path: /opt/homebrew/Cellar/jank/0.1/bin/../lib/jank/0.1/bin/clang++ (found)
─ ⚠️ configured clang resource dir: /Users/runner/work/jank/jank/compiler+runtime/build/llvm-install/usr/local/lib/clang/22 (not found)
─ ✅ runtime clang resource dir: /opt/homebrew/Cellar/jank/0.1/lib/jank/0.1/lib/clang/22 (found)

─ jank runtime ─────────────────────────────────────────────────────────────────────────────────────
─ ✅ jank runtime initialized
─ ✅ jank pch path: /Users/jeaye/.cache/jank/arm64-apple-darwin25.0.0-f33ec85999b436c281e9fba631425b57189670f96ba3166f2d327cd1543b516d (found)
─ ✅ jank can jit compile c++
─ ✅ jank can aot compile working binaries

─ support ──────────────────────────────────────────────────────────────────────────────────────────
If you're having issues with jank, please either ask the jank community on the Clojurians Slack or report the issue on Github.

─ Slack: https://clojurians.slack.com/archives/C03SRH97FDK
─ Github: https://github.com/jank-lang/jank
```

## How to read the output
jank's health check will provide essential information about the jank
installation, Clang installation, and current system. In general, if you don't
see any ❌ then you're good to go. However, for more subtle issues, you may need
to look at the particular paths which jank has determined to ensure they match
up with your expectations.

Either way, when you're reporting a bug or submitting system information,
including your health check output is very useful.
