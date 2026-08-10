# analyze/invalid-cpp-operator-call
Invalid C++ operator call.

## Additional explanation
This happens when a C++ operator form is called with the wrong number of
arguments or with operand types that do not support that operator.

## Mitigations
Call the operator with the required number of arguments, and make sure the
operand types support it.
