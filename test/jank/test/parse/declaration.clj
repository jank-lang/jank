(ns jank.test.parse.declaration
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

; TODO: Split these into type/binding directories
(deftest declaration
  (doseq [file ["fail-missing-idenfitier"
                "fail-non-identifier-integer"
                "fail-non-identifier-string"
                ; TODO: Generics
                ;"fail-empty-generic-type"
                ;"pass-generic-type-with-parameters"
                "pass-type"
                "pass-binding"
                "pass-generic-binding"]]
    (util/test-file (str "test/parse/declaration/" file))))
