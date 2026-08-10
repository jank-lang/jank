# parse/invalid-reader-gensym
gensym literal is not within a syntax quote.

## Additional explanation
This happens when a gensym-style symbol ending in `#` is used outside a
syntax-quoted form.

## Mitigations
Use gensym literals only inside syntax quote, or replace them with an ordinary
symbol.
