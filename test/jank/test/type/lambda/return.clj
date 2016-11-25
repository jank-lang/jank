(ns jank.test.type.lambda.return
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest lambda-return
  (doseq [file ["fail-lambda-wrong-type"
                "fail-no-return"
                "fail-unknown-type"
                "fail-unknown-value"
                "fail-wrong-param-type"
                "fail-wrong-type"
                "fail-first-class-lambda-wrong-return-type"
                "fail-first-class-lambda-wrong-parameter-type"
                "pass-lambda"
                "pass-normal"
                "pass-param"
                "pass-void-no-return"
                "pass-void-incomplete-if"
                "pass-void-wrong-type"]]
    (util/test-file (str "test/type/lambda/return/" file))))
