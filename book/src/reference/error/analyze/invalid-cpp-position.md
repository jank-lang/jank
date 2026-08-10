# analyze/invalid-cpp-position
Unable to use this form as a first-class value. It needs to be called directly.

## Additional explanation
This happens when a C++ operator or member-style form is used as a plain value
instead of being called directly.

## Mitigations
Call the form directly in place, such as `(.member obj ...)`, instead of trying
to pass it around as a value.
