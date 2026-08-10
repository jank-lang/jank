# runtime/invalid-unbox
This happens when a boxed foreign value is unboxed as the wrong type.

## Mitigations
Make sure the unbox operation uses the same type that was originally boxed. The
jank error output should provide you with source information, as well as the
expected type for the given box.
