(ns jank.test.parse.comment
  (:refer-clojure :exclude [comment])
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest comment
  (doseq [file ["multi-line/fail-double-close"
                "multi-line/fail-no-close"
                "multi-line/pass-normal"
                "multi-line/pass-parens"
                "multi-line/pass-quotes"
                "multi-line/pass-unicode"
                "nested/fail-multi-line-multi-end"
                "nested/fail-no-close"
                "nested/fail-single-line-multi-end"
                "nested/pass-multi-line"
                "nested/fail-multi-line-multi-start"
                "nested/pass-single-line"
                "nested/fail-single-line-multi-start"
                "single-line/fail-double-close"
                "single-line/fail-no-close"
                "single-line/pass-multiple-in-one-file"
                "single-line/pass-normal"
                "single-line/pass-parens"
                "single-line/pass-quotes"
                "single-line/pass-single-unmatched-quote"
                "single-line/pass-unicode"]]
    (util/test-file (str "test/parse/comment/" file))))
