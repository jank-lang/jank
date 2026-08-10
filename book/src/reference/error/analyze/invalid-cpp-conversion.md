# analyze/invalid-cpp-conversion

This happens when analyzed code returns a native C++ value that jank cannot convert into a jank object.

## Mitigations
Return a jank object instead. You can use an opaque box. You can define a trait conversion. You can also build a jank object from the native value's members.
