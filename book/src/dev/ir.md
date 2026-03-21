# IR reference

```clojure
{:name 'my-fn
 :blocks [{:name 'entry
           :instructions [{:name 'v0 :op :literal :value 42}
                          {:name 'v1 :op :var-deref :value 'clojure.core/println}
                          {:name 'v2 :op :dynamic-call :fn 'v1 :args ['v0]}
                          {:op :ret :value 'v2}]}]}
```

## Resources
* https://gist.github.com/pizlonator/cf1e72b8600b1437dda8153ea3fdb963
* https://capra.cs.cornell.edu/bril/lang/ssa2.html
