# parse/duplicate-keys-in-map
Duplicate keys in map literals are not allowed.

## Additional explanation
This happens when the same key appears more than once in a map literal. For
example:

```clojure
{:a 1
 :a 2}
```

## Mitigations
Remove or rename the duplicate key so each key appears only once.
