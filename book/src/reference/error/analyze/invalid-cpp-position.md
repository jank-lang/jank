# analyze/invalid-cpp-position

This happens when a C++ operator or member-style form is used as a plain value instead of being called directly.

## Mitigations
Call the form directly in place, such as `(.member obj ...)`. Do not try to pass it around as a value.
