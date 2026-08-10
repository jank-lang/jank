# analyze/invalid-cpp-unbox
Invalid C++ unbox.

## Additional explanation
This happens when `cpp/unbox` is malformed, is given the wrong number of
arguments, uses a non-pointer target type, or is applied to something that is
not a boxed jank object.

## Mitigations
Call `cpp/unbox` with a pointer target type and a compatible boxed jank value.
