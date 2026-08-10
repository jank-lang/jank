# parse/duplicate-keys-in-map

This happens when the same key appears more than once in a map literal. For example:

```clojure
{:a 1
 :a 2}
```

## Mitigations
Either remove the duplicate key or rename it so each key appears only once.
