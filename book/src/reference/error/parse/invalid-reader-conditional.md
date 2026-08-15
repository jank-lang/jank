# parse/invalid-reader-conditional

This happens when a reader conditional such as `#?` is not allowed in the current context. It can also happen when it is not followed by a list. Malformed feature or value pairs can trigger it too.

## Mitigations
Use a valid reader conditional list with keyword features and matching forms. Enable reader conditionals if needed. A valid reader conditional looks like this:

```clojure
#?(:jank (println "hi jank!")
   :default (println "hi other clojure!"))
```
