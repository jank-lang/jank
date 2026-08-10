# parse/invalid-shorthand-function-parameter

This happens when a `%` parameter inside `#()` is not one of the supported forms.

## Mitigations
Use one of the supported forms: `%`, `%&`, `%n` where `n` is 1 or greater. `%` means `%1`. `%&` means all parameters as a sequence.
