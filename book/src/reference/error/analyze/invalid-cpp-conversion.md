# analyze/invalid-cpp-conversion
Invalid C++ type returned.

## Additional explanation
This happens when analyzed code returns a native C++ value that jank cannot
convert into a jank object.

## Mitigations
In short, return a jank object. You can get there via opaque boxes, trait
conversions, or manually creating a jank object from the members of your native
value.
