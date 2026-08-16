# aot/unresolved-main

This happens when jank is building an executable and the target module does not define `-main`.

## Mitigations
Define `-main` in the target module. Make sure you are compiling the module that
is meant to be the program entrypoint. A normal `-main` looks like this:

```clojure
(ns my-app.main)

(defn -main [& args]
  )
```

Note that it's `defn -main` and not `defn- main`.
