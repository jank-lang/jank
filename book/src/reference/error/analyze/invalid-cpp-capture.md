# analyze/invalid-cpp-capture

This happens when a local native C++ value is captured in a context where it cannot be safely copied.

## Mitigations
Avoid capturing that value directly. If possible, change the code so the captured value has a copyable type. You can also capture a pointer to it by using `cpp/&` in a `let`. Verify that the pointed-to value will outlive the closure.
