# parse/nested-shorthand-function
Nested `#()` forms are not allowed.

## Additional explanation
This happens when one shorthand anonymous function form is placed inside another
`#()` form. This is a problem since the `%1` and similar argument placeholders
would be ambiguous between the two functions.

## Mitigations
Rewrite the nested shorthand function as a normal `fn`, or move it outside the outer `#()` form.
