# analyze/invalid-cpp-type-position

This happens when a C++ type name is used where jank expects a value instead of a type.

## Mitigations
Move the type form to a type position. Otherwise use a value-producing form instead.

Types cannot be used as first-class values in jank because there is no runtime reflection.
