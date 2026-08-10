# parse/invalid-reader-symbolic-value
Invalid reader symbolic value.

## Additional explanation
This happens when a reader symbolic value is not one of the supported forms, such as `##Inf`, `##-Inf`, or `##NaN`. It may also appear for unsupported built-in tagged-reader cases.

## Mitigations
Use one of the supported symbolic values, or switch to a supported tagged literal form.
