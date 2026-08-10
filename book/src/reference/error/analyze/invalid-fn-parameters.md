# analyze/invalid-fn-parameters
Invalid `fn` parameters.

## Additional explanation
This happens when a function parameter vector is malformed. Common cases include
using something other than symbols, qualifying parameter names, or using `&`
incorrectly.

## Mitigations
Use a parameter vector of unqualified symbols, and use `&` only once immediately
before the variadic parameter name.
