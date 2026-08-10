# parse/invalid-reader-splice

This happens when `#?@` is used where splicing is not allowed. It can also happen when the selected form is not a sequence.

## Mitigations
Use `#?@` only in a place where splicing is allowed. Make sure the selected form is a sequence. A valid reader conditional splice looks like this:

```clojure
(def v [1 2 #?@(:jank [3 4]
                :default [])])
```
