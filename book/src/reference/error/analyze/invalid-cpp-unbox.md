# analyze/invalid-cpp-unbox

This happens when `cpp/unbox` is malformed. It can also happen when it receives the wrong number of arguments. Using a non-pointer target type triggers it too. Applying it to something that is not a boxed jank object also triggers it.

## Mitigations
Call `cpp/unbox` with a pointer target type and a compatible boxed jank value.
