# parse/invalid-reader-comment
Invalid reader comment.

## Additional explanation
This happens when `#_` is not followed by a form to skip.

## Mitigations
Place a form immediately after `#_`, or remove the reader comment. For example:

```clojure
(let [a (get-a!)]
  #_(println :a a)
  a)
