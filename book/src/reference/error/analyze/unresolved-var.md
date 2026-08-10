# analyze/unresolved-var

This happens when `(var some-symbol)` refers to a var that jank cannot resolve. The same error can appear for `#'some-symbol`.

## Mitigations
Make sure the var exists. Check the spelling. Make sure it is available in the current namespace or use a fully qualified name. You can write a fully qualified var reference as `#'some.ns/foo`.
