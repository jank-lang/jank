(ns jank.test.parse.paren
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest paren
  (doseq [file ["match/fail-close-nothing-else"
                "match/fail-multiple-close-nothing-else"
                "match/fail-multiple-open-nothing-else"
                "match/fail-open-nothing-else"]]
    (util/test-file (str "test/parse/paren/" file))))
