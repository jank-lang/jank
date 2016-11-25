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
