# analyze/invalid-cpp-dsl

This happens when a `#cpp` type form is malformed. Common cases include invalid modifiers. They also include invalid template usage. Malformed function type forms can trigger it. Malformed array type forms can trigger it. Using a value form where a type form is required can trigger it too.

## Mitigations
Rewrite the `#cpp` form so each part is a valid C++ type description. Make sure each part appears in the correct position.

For full documentation on the C++ DSL, see [here](/cpp-interop/dsl.md).
