# parse/invalid-syntax-unquote-splice

This happens when `~@` is used somewhere syntax-quote splicing is not allowed.

## Mitigations
Use `~@` only inside a sequence position where splicing makes sense.
