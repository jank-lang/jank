(ns jank.test.parse.comment.multi-line
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest comment-multi-line
  (doseq [file ["fail-double-close"
                "fail-no-close"
                "pass-normal"
                "pass-parens"
                "pass-quotes"
                "pass-single-unmatched-quote"
                "pass-unicode"]]
    (util/test-file (str "test/parse/comment/multi-line/" file))))
