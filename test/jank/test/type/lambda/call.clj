(ns jank.test.type.lambda.call
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest lambda-call
  (doseq [file ["fail-invalid-lambda"
                "fail-invalid-param-type"
                "fail-too-few-params"
                "fail-too-many-params"
                "fail-recursion-undeduced"
                "fail-call-non-lambda-from-return"
                "pass-chain"
                "pass-empty"
                "pass-lambda-call-param"
                "pass-print"
                "pass-print-primitive"
                "pass-lambda-directly"
                "pass-lambda-directly-with-arguments"
                "pass-lambda-return"
                "pass-lambda-return-with-arguments"
                "pass-deduced-direct-as-parameter"
                "pass-deduced-direct-as-binding"
                "pass-deduced-auto-direct-as-binding"
                "pass-recursion"
                "pass-recursion-undeduced-call-overload"]]
    (util/test-file (str "test/type/lambda/call/" file))))
