(ns jank.test.type.lambda.bind
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest lambda-bind
  (doseq [file ["fail-incorrect-type"
                "fail-invalid-param-type"
                "fail-invalid-return-type"
                "fail-same-param-name"
                "pass-with-type"
                "pass-simple"
                "pass-call"
                "pass-nested"]]
    (util/test-file (str "test/type/lambda/bind/" file))))
