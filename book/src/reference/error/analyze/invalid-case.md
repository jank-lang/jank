# analyze/invalid-case
Invalid `case*`.

## Additional explanation
This happens when the low-level `case*` form is used directly with missing arguments or arguments of the wrong type.

## Mitigations
Prefer the normal `case` macro instead of calling `case*` directly. If you do use `case*`, make sure each required argument is present and has the expected type.
