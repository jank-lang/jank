(ns jank.test.type.declaration.binding
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest declaration-binding
  ; TODO
  (doseq [file ["fail-mismatched-types"
                "fail-unknown-type"
                "pass-lambda"
                "pass-multiple"
                "pass-normal"]]
    (util/test-file (str "test/type/declaration/binding/" file))))
