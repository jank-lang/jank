# analyze/invalid-var-reference

This happens when `(var ...)` is malformed. It may have the wrong number of arguments. The argument may not be a symbol.

## Mitigations
Use `(var some-symbol)` with exactly one symbol argument.
