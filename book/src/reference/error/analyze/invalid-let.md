# analyze/invalid-let

This happens when a `let` binding form is malformed. The binding form may be missing. It may not be a vector. It may have an odd number of entries. The binding names may be invalid.

## Mitigations
Use a binding vector with pairs of unqualified symbol names and values.
