# runtime/invalid-cpp-eval
This happens when runtime C++ evaluation fails. The provided C++ may be invalid
or the JIT toolchain may have failed while compiling it.

## Mitigations
Check the accompanying compiler output first. Then fix the C++ source or the
local Clang and JIT setup before trying again.
