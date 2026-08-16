# analyze/unresolved-symbol

This happens when a symbol is not a local. It may also not be a named recursion target. In that case jank cannot resolve it to a known var or supported C++ global.

## Mitigations
Check the spelling. Check the namespace. Check the scope. Make sure any required namespace has been loaded or aliased.
