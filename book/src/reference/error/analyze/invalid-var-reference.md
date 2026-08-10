# analyze/invalid-var-reference
Invalid var reference.

## Additional explanation
This happens when `(var ...)` is malformed, such as when it has the wrong number
of arguments or the argument is not a symbol.

## Mitigations
Use `(var some-symbol)` with exactly one symbol argument.
