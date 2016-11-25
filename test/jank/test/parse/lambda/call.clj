(ns jank.test.parse.lambda.call
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest lambda-call
  (doseq [file ["fail-non-function-literal"]]
    (util/test-file (str "test/parse/function/call/" file))))
