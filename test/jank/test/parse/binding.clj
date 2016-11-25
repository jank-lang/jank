(ns jank.test.parse.binding
  (:refer-clojure :exclude [binding])
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest binding
  (doseq [file ["fail-missing-value"
                "pass-builtin-function"
                "pass-builtin-identifier"
                "pass-builtin-literal"
                "pass-builtin-parameter"]]
    (util/test-file (str "test/parse/binding/" file))))
