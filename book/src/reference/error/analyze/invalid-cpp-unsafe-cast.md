# analyze/invalid-cpp-unsafe-cast
Invalid C++ unsafe-cast.

## Additional explanation
This happens when `cpp/unsafe-cast` is malformed or when even an unsafe C-style
cast cannot be formed for the given types.

## Mitigations
Call `cpp/unsafe-cast` with exactly a target type and a value, and make sure the
cast is actually possible.
