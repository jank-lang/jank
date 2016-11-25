(ns jank.test.type.lambda.nest
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest lambda-nest
  (doseq [file ["fail-multiple-inner-definition"
                "pass-capture-params"
                "pass-define"
                "pass-overload-inner"
                "pass-overload-outer-call-outer"
                "pass-overload-outer"
                "pass-overload-self"
                ; TODO: Fix
                ;"pass-redefine-outer"
                ;"pass-redefine-self"
                ]]
    (util/test-file (str "test/type/lambda/nest/" file))))
