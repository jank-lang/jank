(ns jank.test.parse.if
  (:refer-clojure :exclude [if])
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))
(deftest if
  (doseq [file ["fail-without-both"
                "fail-too-many-branches"
                "pass-with-literal"
                "pass-without-else"
                "pass-with-predicate"]]
    (util/test-file (str "test/parse/if/" file))))
