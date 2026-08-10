# parse/invalid-syntax-unquote
Invalid syntax unquote.

## Additional explanation
This happens when an unquote form such as `~` is missing its value or is otherwise used incorrectly.

## Mitigations
Use `~` only with a following form in a valid syntax-quote context.
