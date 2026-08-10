# analyze/invalid-cpp-dsl
Invalid C++ type form.

## Additional explanation
This happens when a `#cpp` type form is malformed. Common cases include
invalid modifiers, invalid template usage, malformed function or array type
forms, and mixing value forms where a type form is required.

## Mitigations
Rewrite the `#cpp` form so each part is a valid C++ type description and
appears in the correct position.

For full documentation on the C++ DSL, see [here](/cpp-interop/dsl.md).
