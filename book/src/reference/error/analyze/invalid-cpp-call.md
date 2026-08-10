# analyze/invalid-cpp-call
Invalid C++ call.

## Additional explanation
This happens when jank cannot form a valid indirect or generic C++ call, such as
a call through a function pointer, functor, or other callable-like value.

## Mitigations
Make sure the target is actually callable and that the argument list matches the
available call operator or function type.
