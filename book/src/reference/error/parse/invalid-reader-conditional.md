# parse/invalid-reader-conditional
Invalid reader conditional.

## Additional explanation
This happens when a reader conditional such as `#?` is not allowed in the
current context, is not followed by a list, or contains malformed feature/value
pairs.

## Mitigations
Use a valid reader conditional list with keyword features and matching forms,
and enable reader conditionals if needed. A valid reader conditional looks like
this:

```clojure
#?(:jank (println "hi jank!")
   :default (println "hi other clojure!"))
```
