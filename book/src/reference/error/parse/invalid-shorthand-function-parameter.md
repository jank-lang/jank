# parse/invalid-shorthand-function-parameter
Invalid shorthand function parameter.

## Additional explanation
This happens when a `%` parameter inside `#()` is not one of the supported forms.

## Mitigations
Use `%` (which means `%1`), `%&` (which means all parameters as a sequence), or `%n` where `n` is 1 or greater.
