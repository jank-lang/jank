# analyze/invalid-def
Invalid `def`.

## Additional explanation
This happens when a `def` form is malformed, such as when it is missing its name, has extra forms, or uses something other than an unqualified symbol as the name.

## Mitigations
Write `def` as `(def name value)` or `(def name)`, and make sure `name` is an unqualified symbol.
