# analyze/invalid-cpp-function-call
Invalid C++ function call.

## Additional explanation
This happens when a direct C++ function-style call cannot be resolved, such as
when the target is not callable, no overload matches, or the requested function
cannot be found.

## Mitigations
Check the function name, template arguments, and call arguments, and make sure a
matching callable target exists.
