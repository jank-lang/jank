# analyze/invalid-cpp-member-call

This happens when a `.member` call is missing its target object. It can also happen when the target type does not have a matching callable member function.

## Mitigations
Pass the target object first. Make sure the member function exists for that type and argument list. The member function must be public.
