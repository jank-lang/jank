# analyze/invalid-cpp-new

This happens when `cpp/new` is missing the type it should allocate.

## Mitigations
Pass the type to allocate, followed by any constructor arguments.
