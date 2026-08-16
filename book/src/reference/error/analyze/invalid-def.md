# analyze/invalid-def

This happens when a `def` form is malformed. It may be missing its name. It may have extra forms. The name may not be an unqualified symbol.

## Mitigations
Write `def` as `(def name value)` or `(def name)`. Make sure `name` is an unqualified symbol.
