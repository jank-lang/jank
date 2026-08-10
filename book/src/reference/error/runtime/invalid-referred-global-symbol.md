# runtime/invalid-referred-global-symbol
This happens when a referred C++ global name is malformed.

## Mitigations
Use a simple symbol name for the referred C++ global. Avoid namespace-qualified
names in that position.
