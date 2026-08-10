# analyze/invalid-cpp-box

This happens when `cpp/box` is malformed. It can also happen when it receives the wrong number of arguments or when the value cannot be boxed this way.

`cpp/box` must receive a raw pointer value. The pointed-to value must outlive the returned opaque box.

## Mitigations
Call `cpp/box` with exactly one suitable pointer value.
