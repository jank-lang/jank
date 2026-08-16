# analyze/invalid-if

This happens when an `if` form is malformed. It may be missing the `then` branch. It may also have too many forms.

## Mitigations
Write `if` as `(if test then)` or `(if test then else)`. If you need multiple forms in the `then` branch or the `else` branch, wrap them in a `do` form.
