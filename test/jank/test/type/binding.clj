(ns jank.test.type.binding
  (:refer-clojure :exclude [binding])
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest binding
  (doseq [file ["fail-lambda-with-incompatible-type"
                "fail-identifier-with-incompatible-type"
                "fail-incompatible-value"
                "fail-multiple-definition-different-type"
                "fail-multiple-definition-same-type"
                "fail-unknown-type"
                "fail-unknown-value-identifier"
                "fail-outside-lambda"
                "pass-deduce-type"
                "pass-proper-types"
                "pass-within-lambda"]]
    (util/test-file (str "test/type/binding/" file))))
