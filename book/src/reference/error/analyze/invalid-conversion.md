# analyze/invalid-conversion
Invalid trait conversion.

## Additional explanation
This happens when native C++ evaluation produces a native value which jank cannot
convert into a jank object.

## Mitigations
You can either wrap the native value in an opaque box or provide a converstion
trait so that jank knows how to convert from your native type into a jank
object.

You might also consider manually constructing your own jank objects based on the
members inside the native value. For more information on this, see
[here](/cpp-interop/native-values.md).
