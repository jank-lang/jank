# parse/duplicate-items-in-set
Duplicate items in set literals are not allowed.

## Additional explanation
This happens when the same item appears more than once in a set literal. For
example:

```clojure
#{:foo :foo}
```

Note that this applies even to function calls which might produce different
output, such as:

```clojure
#{(rand) (rand)}
```

## Mitigations
Remove the duplicate item so each set element appears only once. If your use
case is similar to the `(rand)` example above, pull the values into a `let`
first and then use those locals to create your set.

```clojure
(let [a (rand)
      b (rand)]
 #{a b})
```
