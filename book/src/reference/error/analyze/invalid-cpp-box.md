# analyze/invalid-cpp-box
Invalid C++ box.

## Additional explanation
This happens when `cpp/box` is malformed, is given the wrong number of
arguments, or is used on a value that cannot be boxed this way.

Note that `cpp/box` must be given a raw pointer value. Also, the lifetime of the
pointed-to value must be able to outlive the lifetime of the returned opaque
box.

## Mitigations
Call `cpp/box` with exactly one suitable pointer value.
