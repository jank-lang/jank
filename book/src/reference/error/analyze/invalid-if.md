# analyze/invalid-if
Invalid `if`.

## Additional explanation
This happens when an `if` form is malformed, such as when it is missing the
`then` branch or has too many forms.

## Mitigations
Write `if` as `(if test then)` or `(if test then else)`. If you need multiple
forms within the `then` or `else` branch, wrap them in a `do` form.
