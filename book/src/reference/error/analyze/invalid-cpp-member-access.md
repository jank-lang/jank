# analyze/invalid-cpp-member-access
Invalid C++ member access.

## Additional explanation
This happens when a `.-member` form is malformed, is missing its target object,
has extra arguments, or refers to a field that does not exist on the target
type.

## Mitigations
Pass exactly one target object and make sure the named member exists and is
accessible on that type. Also make sure that the member is public.
