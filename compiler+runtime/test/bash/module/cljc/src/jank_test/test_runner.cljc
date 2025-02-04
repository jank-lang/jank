(ns jank-test.test-runner
  (:require jank-test.a
            jank-test.cljc))

(defn -main []
  (assert (= "a.jank" (jank-test.a/-main)) ".jank overrides .cljc")
  (assert (= :jank (jank-test.cljc/-main)) "cljc files use :jank reader conditional feature")
  ;; exit code not reliable enough https://github.com/jank-lang/jank/issues/181
  (println :success))
