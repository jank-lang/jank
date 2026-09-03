# runtime/invalid-unbox
This happens when a boxed native pointer is unboxed as the wrong type.

## Mitigations
Make sure the `cpp/unbox` operation uses the same type that was originally boxed. The
jank error output should provide you with source information, as well as the
expected type for the given box.

Note that opaque boxes always need to store raw pointers. Additional qualifiers
like `const` do matter, when it comes to type comparison. For more information
on opaque boxes, see [here](/cpp-interop/native-values.md).
