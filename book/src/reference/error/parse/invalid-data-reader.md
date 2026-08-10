# parse/invalid-data-reader
Invalid data reader.

## Additional explanation
This happens when tagged-literal data reader configuration is invalid, such as when `*data-readers*` is not a map or a data reader is not a function.

## Mitigations
Fix the tagged-literal reader configuration before reading the form again.
