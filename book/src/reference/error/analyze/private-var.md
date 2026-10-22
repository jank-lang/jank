# analyze/private-var

This happens when a var that is referenced exists, but is private. A var is
determined to be private if its metadata has a truthy value for the `:private`
key.

## Mitigations
Generally, private vars should not be accessed, so first consider if there is a
public API which provides what you need.

If you must, you can access the var directly with `#'some.ns/foo`. From there,
you could derefence it to get the value with `@#'some.ns/foo`. This mitigates
the private check altogether.
