# analyze/macro-expansion-exception
Macro expansion exception.

## Additional explanation
This happens when a macro throws an exception or another error while jank is
expanding it.

## Mitigations
Inspect the macro call and the macro itself. Reduce the input to a smaller
example and fix the failing expansion path.
