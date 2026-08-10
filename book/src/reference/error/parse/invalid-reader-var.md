# parse/invalid-reader-var
Invalid reader var reference.

## Additional explanation
This happens when `#'` is not followed by a symbol. For example, `#'foo` is
valid, assuming that `foo` can be resolved in the current namespace. However,
`#'123` is not valid.

## Mitigations
Put a symbol immediately after `#'`.
