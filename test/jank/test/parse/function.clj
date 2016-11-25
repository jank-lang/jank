(ns jank.test.parse.function
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest function-calls
  (doseq [file ["fail-non-function-literal"]]
    (util/test-file (str "test/parse/function/call/" file))))
