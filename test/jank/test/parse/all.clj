(ns jank.test.parse.all
  (:require [clojure.test :refer :all]
            [jank.test.bootstrap :refer :all :refer-macros :all]))

(def error #"parse error:")

(defn test-file [file]
  (let [full-file (str file ".jank")]
    (println "[parse] testing" full-file)
    (if (should-fail? full-file)
      (is (thrown-with-msg? AssertionError
                            error
                            (valid-parse? full-file)))
      (is (valid-parse? full-file)))))

(deftest new-expressions
  (doseq [file ["pass-multiple-types"
                "pass-no-values"
                "pass-function-parameter"
                "pass-generic-type"
                "pass-no-type"
                "pass-one-value"
                "pass-value-expressions"]]
    (test-file (str "test/parse/new/" file))))
