# analyze/invalid-cpp-member-access

This happens when a `.-member` form is malformed. It can also happen when the target object is missing. Extra arguments can trigger it too. A missing field on the target type can also trigger it.

## Mitigations
Pass exactly one target object. Make sure the named member exists on that type. Make sure it is accessible. The member must be public.
