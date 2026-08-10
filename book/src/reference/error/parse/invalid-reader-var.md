# parse/invalid-reader-var

This happens when `#'` is not followed by a symbol. For example, `#'foo` is valid if `foo` resolves in the current namespace. `#'123` is not valid.

## Mitigations
Put a symbol immediately after `#'`.
