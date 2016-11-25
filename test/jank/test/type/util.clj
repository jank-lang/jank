(ns jank.test.type.all
  (:require [clojure.test :refer :all]
            [jank.test.bootstrap :refer :all :refer-macros :all]))

(def error #"type error:")

(defn test-file [file]
  (let [full-file (str file ".jank")]
    (println "[type] testing" full-file)
    (if (should-fail? full-file)
      (is (thrown-with-msg? AssertionError
                            error
                            (valid-type? full-file)))
      (is (valid-type? full-file)))))


(deftest new-expressions
  (doseq [file ["fail-incorrect-value-types"
                "fail-no-values"
                "fail-not-enough-values"
                "fail-too-many-types"
                "fail-too-many-values"
                "fail-unknown-type"
                "pass-as-parameter"
                "pass-constructor"
                "pass-correct-values"
                "pass-value-expressions"
                "pass-binding-explicit"
                "pass-binding-implicit"]]
    (test-file (str "test/type/new/" file))))
