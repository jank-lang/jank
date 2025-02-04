(ns jank-test.test-runner
  (:require jank-test.a))

(defn -main []
  (assert (= "a.jank" (jank-test.a/-main)))
  ;; exit code not reliable enough https://github.com/jank-lang/jank/issues/181
  (println :success))
