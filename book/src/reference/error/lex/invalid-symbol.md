# lex/invalid-symbol

This most often means the symbol starts with `/`. That is not allowed.

## Mitigations
Either remove the leading `/` or add a namespace before it, such as `my.ns/name`.
