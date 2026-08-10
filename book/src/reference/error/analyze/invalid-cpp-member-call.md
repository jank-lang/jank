# analyze/invalid-cpp-member-call
Invalid C++ member function call.

## Additional explanation
This happens when a `.member` call is missing its target object or when the
target type does not have a matching callable member function.

## Mitigations
Pass the target object first. Make sure the member function exists for that
type and argument list. Also be sure that the member function is public.
