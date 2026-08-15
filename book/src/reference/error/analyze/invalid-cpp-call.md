# analyze/invalid-cpp-call

This happens when jank cannot form a valid indirect or generic C++ call. That can include calls through function pointers. It can also include calls through functors or other callable values.

## Mitigations
Make sure the target is actually callable. Make sure the argument list matches an available call operator or function type.
