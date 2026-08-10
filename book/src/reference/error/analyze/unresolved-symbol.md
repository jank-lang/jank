# analyze/unresolved-symbol
Unresolved symbol.

## Additional explanation
This happens when a symbol is not a local, not a named recursion target, and
cannot be resolved to a known var or supported C++ global.

## Mitigations
Check the spelling, namespace, and scope of the symbol, and make sure any
required namespace has been loaded or aliased.
