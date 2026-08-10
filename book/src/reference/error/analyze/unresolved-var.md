# analyze/unresolved-var
Unresolved var.

## Additional explanation
This happens when `(var some-symbol)` refers to a var that jank cannot resolve.
This can also happen for `#'some-symbol`, which parses into the same thing.

## Mitigations
Make sure the var exists, is spelled correctly, and is available in the current
namespace or via a fully qualified name. Note that jank resolves `#'foo` in the
current namespace, but that you can fully qualify it if the var lives in another
namespace with `#'some.ns/foo`.
