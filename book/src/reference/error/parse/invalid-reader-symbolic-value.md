# parse/invalid-reader-symbolic-value

This happens when a reader symbolic value is not one of the supported forms. Supported forms include `##Inf`, `##-Inf`, `##NaN`. It may also appear for unsupported built-in tagged-reader cases.

## Mitigations
Use one of the supported symbolic values. Otherwise switch to a supported tagged literal form.
