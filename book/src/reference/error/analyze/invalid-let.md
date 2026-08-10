# analyze/invalid-let
Invalid `let`.

## Additional explanation
This happens when a `let` binding form is malformed, such as when the binding
form is missing, is not a vector, has an odd number of entries, or uses invalid
binding names.

## Mitigations
Use a binding vector with pairs of unqualified symbol names and values.
