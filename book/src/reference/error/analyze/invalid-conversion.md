# analyze/invalid-conversion

This happens when native C++ evaluation produces a value that jank cannot convert into a jank object.

## Mitigations
Either wrap the native value in an opaque box or define a conversion trait for that type.

You can also construct a jank object manually from the native value. For more information, see [native values](/cpp-interop/native-values.md).
