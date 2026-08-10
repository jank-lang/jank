# parse/invalid-reader-splice
Invalid reader splice.

## Additional explanation
This happens when `#?@` is used where splicing is not allowed, or when it is
applied to a value that is not a sequence.

## Mitigations
Use `#?@` only in a place where splicing is allowed, and make sure the selected
form is a sequence. A valid reader conditional splice looks like this:

```clojure
(def v [1 2 #?@(:jank [3 4]
                :default [])])
```
