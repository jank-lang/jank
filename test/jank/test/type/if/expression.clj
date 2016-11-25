(ns jank.test.type.if.expression
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest if-expression
  (doseq [file ["fail-different-types"
                "fail-invalid-param-type"
                "fail-without-else"
                "fail-nested-without-else"
                "pass-matching-types"
                "pass-if-as-condition"
                "pass-nested"
                ]]
    (util/test-file (str "test/type/if/expression/" file))))
