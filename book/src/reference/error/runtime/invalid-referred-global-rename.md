# runtime/invalid-referred-global-rename

This happens when a requested local rename for a referred C++ global would conflict with an existing name in the namespace.

## Mitigations
Choose a different local name that does not collide with an existing var.
