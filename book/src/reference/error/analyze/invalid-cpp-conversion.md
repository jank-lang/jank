# analyze/invalid-cpp-conversion
This happens when there's an unsupported implicit conversion needed between two
types. This can happen between two native types, such as when you try to use a
native value for an `if` condition and it can't implicitly convert to `bool`.

This can also happen for unsupported trait conversions. In that case, a jank object is
expected and a native value is provided, but there is no conversion trait which
allows jank to get from the native value to the jank object.

## Mitigations
If you're implicitly casting between two native types, you'll need to sort out
how that can be done more explicitly.

If you're converting a native value into a jank object, you have a few options.

1. You can wrap the native value in an opaque box, if its lifetime allows that.
2. You can define a conversion trait for your native type.
3. You can also build a jank object from the native value's members.

For more information on all of this see [here](/cpp-interop/native-values.md).
