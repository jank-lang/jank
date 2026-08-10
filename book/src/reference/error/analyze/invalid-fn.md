# analyze/invalid-fn
Invalid `fn`.

## Additional explanation
This happens when an `fn` form has an invalid overall shape, such as malformed arities, duplicate arities, or invalid variadic usage.

## Mitigations
Rewrite the function so each arity has a valid parameter vector and body, and
make sure any variadic arity is well formed. Note that `fn` comes in two forms.

### Single arity
```clojure
(fn foo [a1 a2]
  (println a1 a2))
```

### Multi-arity
```clojure
(fn foo
  ([a1]
   (println a1))
  ([a1 a2]
   (println a1 a2)))
```
