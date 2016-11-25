(ns jank.test.parse.ident
  (:require [clojure.test :refer :all]
            [jank.test.parse.all :as util]))

(deftest ident
  (doseq [file ["ascii/fail-bad-chars"
                "ascii/pass-good-chars"
                "ascii/pass-true-false"
                "unicode/pass-all-good"]]
    (util/test-file (str "test/parse/ident/" file))))
