# parse/invalid-reader-gensym

This happens when a gensym-style symbol ending in `#` is used outside a syntax-quoted form.

## Mitigations
Use gensym literals only inside syntax quote. Otherwise replace them with an ordinary symbol.
