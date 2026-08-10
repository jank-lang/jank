# analyze/invalid-cpp-symbol

This happens when a C++ symbol form is malformed. It may refer to a namespace as a value. It may use an invalid dotted name. It may use constructor syntax on something that is not a type.

## Mitigations
Rewrite the symbol as a valid C++ namespace, type, function, member name. Only use constructor syntax with actual types.
