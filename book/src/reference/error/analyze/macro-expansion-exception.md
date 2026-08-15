# analyze/macro-expansion-exception

This happens when a macro throws an exception or another error while jank expands it.

## Mitigations
Inspect the macro call. Inspect the macro itself. Reduce the input to a smaller example and fix the failing expansion path.
