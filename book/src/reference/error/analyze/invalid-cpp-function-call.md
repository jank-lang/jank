# analyze/invalid-cpp-function-call

This happens when a direct C++ function-style call cannot be resolved. The target may not be callable. No overload may match. The requested function may not exist.

## Mitigations
Check the function name. Check the template arguments. Check the call arguments. Make sure a matching callable target exists.
