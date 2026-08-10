# analyze/invalid-cpp-cast
Invalid C++ cast.

## Additional explanation
This happens when `cpp/cast` is malformed or when the requested cast is not
allowed for the source and target types.

## Mitigations
Call `cpp/cast` with exactly a target type and a value, and choose a conversion
that is valid for those types.

Note that `cpp/cast` does the equivalent of a C++ `static_cast`. If you need
something more than that, use a `cpp/unsafe-cast` instead.
