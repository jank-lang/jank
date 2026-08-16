# parse/invalid-reader-comment

This happens when `#_` is not followed by a form to skip.

## Mitigations
Either place a form immediately after `#_` or remove the reader comment. For example:

```clojure
(let [a (get-a!)]
  #_(println :a a)
  a)
```
