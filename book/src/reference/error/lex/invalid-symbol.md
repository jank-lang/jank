# lex/invalid-symbol
Invalid symbol.

## Additional explanation
This currently most commonly means the symbol starts with `/`, which is not allowed.

## Mitigations
Remove the leading `/`, or add a namespace before it, such as `my.ns/name`.
