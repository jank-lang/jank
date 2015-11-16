(ns jank.parse-test
  (:require [clojure.test :refer :all]
            [jank.bootstrap :refer :all]))

(deftest parse
  (is (thrown? AssertionError (valid-type? "test.jank")))
  )
