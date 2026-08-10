# analyze/invalid-recur-from-try

This happens when `recur` appears within a `try` path. The same rule applies inside `catch` and `finally`.

## Mitigations
Restructure the code so the `recur` happens outside the `try` form.
