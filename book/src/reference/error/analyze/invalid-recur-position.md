# analyze/invalid-recur-position

This happens when `recur` is used outside the tail position of a `fn` or `loop`.

## Mitigations
Move the `recur` call to the tail position of the enclosing `fn` or `loop`.
