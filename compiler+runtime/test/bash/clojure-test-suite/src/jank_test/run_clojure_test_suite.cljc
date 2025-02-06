(ns jank-test.run-clojure-test-suite
  (:require [clojure.test :as t]))

(def namespaces
  '[
    clojure.core-test.any-qmark
    clojure.core-test.first
    ])

(defn -main []
  (when (seq namespaces)
    (apply require namespaces)
    (assert (t/successful? (apply t/run-tests namespaces))))
  (println :clojure-test-suite-successful))
