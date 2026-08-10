# analyze/invalid-cpp-raw
Invalid C++ raw.

## Additional explanation
This happens when `cpp/raw` is missing its string argument, has extra arguments,
or is given something other than a string literal of C++ code.

## Mitigations
Call `cpp/raw` with exactly one string literal containing the raw C++ code.
