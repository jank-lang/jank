(ns jank.test.parse.comment.single-line
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest comment-single-line
  (doseq [file ["fail-double-close"
                "fail-no-close"
                "pass-multiple-in-one-file"
                "pass-normal"
                "pass-parens"
                "pass-quotes"
                "pass-single-unmatched-quote"
                "pass-unicode"]]
    (util/test-file (str "test/parse/comment/single-line/" file))))
