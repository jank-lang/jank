(ns jank.test.parse.all
  (:require [clojure.test :refer :all]))

#_(deftest all
  (util/test-files
    "dev/resources/test/parse/"
    [; TODO: Generics
     #".*/declaration/fail-empty-generic-type.*"
     #".*/declaration/pass-generic-type-with-parameters.*"]))
