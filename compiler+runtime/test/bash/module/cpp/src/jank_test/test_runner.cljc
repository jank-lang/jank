(ns jank-test.test-runner
  (:require jank-test.a
            jank-test.b
            jank-test.cpp))

(defn -main []
  (assert (= "a.jank" (jank-test.a/-main)) ".jank overrides .cpp")
  (assert (= "b.cljc" (jank-test.b/-main)) ".cljc overrides .cpp")
  (assert (= :cpp (jank-test.cpp/-main)) "cpp module should be loaded correctly")
  ;; exit code not reliable enough https://github.com/jank-lang/jank/issues/181
  (println :success))
