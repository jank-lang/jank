# analyze/invalid-cpp-capture
Invalid C++ capture.

## Additional explanation
This happens when a local native C++ value is captured in a context where it
cannot be safely copied.

## Mitigations
Avoid capturing that value directly, or change the code so the captured value
has a copyable type. If possible, you could capture a pointer to that value
instead, by using `cpp/&` in a `let` and then capturing that value. However,
it's important to verify the lifetime of the value against the lifetime of the
closure.
