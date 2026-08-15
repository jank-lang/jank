# analyze/invalid-cpp-operator-call

This happens when a C++ operator form is called with the wrong number of arguments. It can also happen when the operand types do not support that operator.

## Mitigations
Call the operator with the required number of arguments. Make sure the operand types support it.
