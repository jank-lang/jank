(ns jank.test.type.lambda.overload
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest lambda-overload
  (doseq [file ["fail-multiple-definition"
                "fail-return-type"
                "pass-different-param-count"
                "pass-same-param-count"]]
    (util/test-file (str "test/type/lambda/overload/" file))))
