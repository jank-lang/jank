(ns jank-test.test-runner
  (:require jank-test.a
            ;;FIXME these names clash via repl_fn
            jank-test.a<a
            jank-test.a-LT-a
            ;;maybe this one too
            jank-test.a.LT-a))

(defn -main []
  (assert (= "a.jank" (jank-test.a/-main)))
  (assert (= "a<a.jank" (jank-test.a<a/-main)))
  (assert (= "a_LT_a.jank" (jank-test.a-LT-a/-main)))
  (assert (= "jank-test.a.LT-a" (jank-test.a.LT-a/-main)))
  )
