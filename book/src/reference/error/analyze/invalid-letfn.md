# analyze/invalid-letfn
Invalid `letfn`.

## Additional explanation
This happens when a `letfn*` binding form is malformed, such as when its
bindings are missing, uneven, use invalid names, or bind something other than
functions.

## Mitigations
Use a binding vector of function-name and function-value pairs, and make sure
each name is an unqualified symbol.
