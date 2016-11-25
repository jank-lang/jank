(ns jank.test.parse.lambda
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest lambda-define
  (doseq [file ["fail-missing-param-name"
                "fail-no-param-list"
                "fail-no-return-type"
                "fail-multiple-return-types"
                "pass-body"
                "pass-empty"
                "pass-primitive"
                "pass-long-form"
                "pass-unicode-short-form"]]
    (util/test-file (str "test/parse/lambda/define/" file))))
