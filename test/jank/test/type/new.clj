(ns jank.test.type.new
  (:refer-clojure :excluce [new])
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest new
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
    (util/test-file (str "test/type/new/" file))))
