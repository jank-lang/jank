# runtime/non-metadatable-value
This happens when metadata is applied to a value type that cannot carry metadata.

## Mitigations
Only use metadata with values that support it. If needed, wrap the value in a
metadatable form first, such as an atom.
