# analyze/invalid-cpp-cast

This happens when `cpp/cast` is malformed or when the requested cast is not allowed for the source type and target type.

## Mitigations
Call `cpp/cast` with exactly a target type and a value. Choose a conversion that is valid for those types.

`cpp/cast` behaves like C++ `static_cast`. If you need a less restricted cast, use `cpp/unsafe-cast`.
