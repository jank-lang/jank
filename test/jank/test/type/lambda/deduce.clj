(ns jank.test.type.lambda.deduce
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest lambda-deduce
  (doseq [file ["fail-mismatched-types"
                "pass-if"
                "pass-void"
                "pass-with-normal-return"
                ; TODO: lambda identifiers
                ;"pass-with-unicode"
                "pass-void-lambda"
                "pass-non-void-lambda"
                "pass-deduced-lambda"]]
    (util/test-file (str "test/type/lambda/deduce/" file))))
