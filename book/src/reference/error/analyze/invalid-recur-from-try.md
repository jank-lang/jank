# analyze/invalid-recur-from-try
`recur` may not be used within a `try`.

## Additional explanation
This happens when `recur` appears within a `try`, `catch`, or `finally` path.

## Mitigations
Restructure the code so the `recur` happens outside the `try` form.
