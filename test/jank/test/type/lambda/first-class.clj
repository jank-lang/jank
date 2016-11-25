(ns jank.test.type.lambda.first-class
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest first-class-lambdas
  (doseq [file ["pass-as-param"
                "fail-incorrect-return-type"
                "fail-incorrect-param-type"
                "pass-simple"
                ; TODO: lambda identifiers
                ;"pass-return-lambda"
                "pass-with-params"
                "pass-higher-order-lambda"]]
    (util/test-file (str "test/type/lambda/first-class/" file))))
