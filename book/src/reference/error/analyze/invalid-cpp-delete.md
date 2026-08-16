# analyze/invalid-cpp-delete

This happens when `cpp/delete` is malformed. It can also happen when it receives the wrong number of arguments or when the value is not a pointer.

## Mitigations
Call `cpp/delete` with exactly one pointer value. The pointed-to value must have been allocated via `cpp/new`.
