# parse/invalid-data-reader

This happens when tagged-literal data reader configuration is invalid. For example, `*data-readers*` may not be a map. A data reader may also not be a function.

## Mitigations
Fix the tagged-literal reader configuration before reading the form again.
