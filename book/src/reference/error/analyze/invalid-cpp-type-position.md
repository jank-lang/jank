# analyze/invalid-cpp-type-position
Invalid position for a C++ type.

## Additional explanation
This happens when a C++ type name is used where jank expects a value instead of a type.

## Mitigations
Move the type form to a type position or use a value-producing form instead.
Note that types cannot be used as first class values in jank, since we have no
runtime reflection.
