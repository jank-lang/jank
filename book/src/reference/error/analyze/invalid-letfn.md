# analyze/invalid-letfn

This happens when a `letfn*` binding form is malformed. The bindings may be missing. They may be uneven. The names may be invalid. The bound values may not be functions.

## Mitigations
Use a binding vector of function-name and function-value pairs. Make sure each name is an unqualified symbol.
