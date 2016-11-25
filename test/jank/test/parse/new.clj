(ns jank.test.parse.new
  (:refer-clojure :exclude [new])
  (:require [clojure.test :refer :all]
            [jank.test.parse.all :as util]))

(deftest new
  (doseq [file ["pass-multiple-types"
                "pass-no-values"
                "pass-function-parameter"
                "pass-generic-type"
                "pass-no-type"
                "pass-one-value"
                "pass-value-expressions"]]
    (util/test-file (str "test/parse/new/" file))))
