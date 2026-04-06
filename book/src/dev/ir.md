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

## Passes
1. Var deref CSE + hoisting
2. Constant propagation
3. Inline core functions
4. DCE
5. CFG simplification
6. Function inlining (small)
7. Constant propagation (again)
8. DCE (again)
9. Global value numbering (GVN)
  * https://bernsteinbear.com/blog/value-numbering/

## References
* riogu
  * https://github.com/riogu/henceforth/blob/main/src/hfs/optimizer.rs
  * https://github.com/riogu/henceforth/blob/main/src/hfs/ir_arena.rs
  * https://github.com/riogu/henceforth/blob/main/src/hfs/hfs_mir.rs
  * https://github.com/riogu/henceforth/blob/main/src/hfs/ir_analysis.rs
