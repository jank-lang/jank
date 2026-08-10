# runtime/module-not-found
This happens when jank cannot find the requested module. Common causes include a
missing dependency, a wrong module path, or asking for a module that is not
available in the current build.

## Mitigations
Make sure the module exists. Then check that the module path, build mode, and
current dependencies make it available to jank.

Note that it's very common to have a module name with a `-` in it, but the
corresponding file system path will use `_` instead. So `my-app.foo-bar` becomes
`my_app/foo_bar.jank` on the file system.
