# analyze/invalid-recur-args

This happens when `recur` is given the wrong number of arguments for the enclosing `fn` or `loop`.

## Mitigations
Pass exactly one new value for each local that the enclosing `fn` arity or `loop` binds.
