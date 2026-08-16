# parse/nested-shorthand-function

This happens when one shorthand anonymous function form is placed inside another `#()` form. The `%1`-style placeholders would be ambiguous between the two functions.

## Mitigations
Either rewrite the nested shorthand function as a normal `fn` or move it outside the outer `#()` form.
