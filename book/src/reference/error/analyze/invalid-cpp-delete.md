# analyze/invalid-cpp-delete
Invalid C++ delete.

## Additional explanation
This happens when `cpp/delete` is malformed, has the wrong number of arguments,
or is given something that is not a pointer.

## Mitigations
Call `cpp/delete` with exactly one pointer value. The pointed-to value must have
been allocated via `cpp/new`.
