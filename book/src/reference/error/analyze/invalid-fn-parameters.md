# analyze/invalid-fn-parameters

This happens when a function parameter vector is malformed. Common cases include non-symbol parameters. Qualified parameter names can also trigger it. Incorrect `&` usage can trigger it too.

## Mitigations
Use a parameter vector of unqualified symbols. Use `&` only once. Put it immediately before the variadic parameter name.
