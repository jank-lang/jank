(ns jank.test.type.lambda.closure
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest lambda-closure
  (doseq [file ["pass-global-from-nested"
                "pass-global"
                "pass-local-from-nested"
                "pass-parameter-from-nested"
                "pass-partial"]]
    (util/test-file (str "test/type/lambda/closure/" file))))
