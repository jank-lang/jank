# analyze/invalid-cpp-symbol
Invalid C++ symbol.

## Additional explanation
This happens when a C++ symbol form is malformed, such as when it refers to a
namespace as a value, uses an invalid dotted name, or uses constructor syntax on
something that is not a type.

## Mitigations
Rewrite the symbol as a valid C++ namespace, type, function, or member name, and
only use constructor syntax with actual types.
