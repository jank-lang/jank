(ns jank.test.parse.string
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest string
  (doseq [file ["pass-numbers"
                "pass-nested-quotes"
                "fail-unmatched-odd"
                "fail-unmatched"
                "pass-multi-line"
                "pass-unicode"]]
    (util/test-file (str "test/parse/string/" file))))
