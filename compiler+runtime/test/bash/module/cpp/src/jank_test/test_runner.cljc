(ns jank-test.test-runner
  (:require jank-test.a
            jank-test.b
            jank-test.cpp))

(defn -main []
  (assert (= "a.cpp" (jank-test.a/-main)) ".cpp overrides .jank")
  (assert (= "b.cpp" (jank-test.b/-main)) ".cpp overrides .cljc")
  (assert (= :cpp (jank-test.cpp/-main)) "cpp module should be loaded correctly")
  ;; exit code not reliable enough https://github.com/jank-lang/jank/issues/181
  (println :success))
