# runtime/unsupported-behavior

This happens when an operation is used on a value whose type does not support that behavior. Common examples include sequence operations, associative lookup, indexed access, comparison, or numeric conversion on the wrong kind of value.

## Mitigations
Use the operation only with values that support it. If needed, convert the value to a compatible type first.
