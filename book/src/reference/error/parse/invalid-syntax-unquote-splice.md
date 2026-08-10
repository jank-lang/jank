# parse/invalid-syntax-unquote-splice
Unquote splice is not within a sequence.

## Additional explanation
This happens when `~@` is used somewhere syntax-quote splicing is not allowed.

## Mitigations
Use `~@` only inside a sequence position where splicing makes sense.
