# parse/odd-entries-in-map
Odd number of entries in map.

## Additional explanation
This happens when a map literal has a key without a corresponding value. For
example:

```clojure
{:a }
```

## Mitigations
Make sure every key in the map is followed by a value.
