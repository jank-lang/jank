# parse/duplicate-items-in-set

This happens when the same item appears more than once in a set literal. For example:

```clojure
#{:foo :foo}
```

This also applies to repeated forms such as:

```clojure
#{(rand) (rand)}
```

## Mitigations
Remove the duplicate item so each set element appears only once. If you need separate computed values, bind them in a `let` first and build the set from those locals.

```clojure
(let [a (rand)
      b (rand)]
 #{a b})
```
