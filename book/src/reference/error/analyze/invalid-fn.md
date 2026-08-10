# analyze/invalid-fn

This happens when an `fn` form has an invalid overall shape. Malformed arities can trigger it. Duplicate arities can trigger it too. Invalid variadic usage can also trigger it.

## Mitigations
Rewrite the function so each arity has a valid parameter vector and body. If you use a variadic arity, make sure it is well formed. `fn` comes in two forms.

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
