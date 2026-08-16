# runtime/module-binary-without-source
This error happens when a required module has no source file on the module path,
but it does have an object file. jank will refuse to load object files for
modules which don't have a corresponding source, since jank is a source-first
language.

## Mitigations
Verify that the module's source is present on the module path.
