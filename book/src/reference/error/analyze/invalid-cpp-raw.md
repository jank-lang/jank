# analyze/invalid-cpp-raw

This happens when `cpp/raw` is missing its string argument. It can also happen when it has extra arguments or when it is given something other than a string literal of C++ code.

## Mitigations
Call `cpp/raw` with exactly one string literal containing the raw C++ code.
