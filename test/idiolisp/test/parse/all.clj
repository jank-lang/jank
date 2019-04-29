(ns idiolisp.test.parse.all
  (:require [clojure.test :refer :all]
            [idiolisp.test.parse.util :as util]))

#_(deftest all
  (util/test-files
    "dev/resources/test/parse/"
    [; TODO: Generics
     #".*/declaration/fail-empty-generic-type.*"
     #".*/declaration/pass-generic-type-with-parameters.*"]))
