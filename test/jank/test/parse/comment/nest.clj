(ns jank.test.parse.comment.nest
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest comment-nest
  (doseq [file ["fail-multi-line-multi-end"
                "fail-no-close"
                "fail-single-line-multi-end"
                "pass-multi-line"
                "fail-multi-line-multi-start"
                "pass-single-line"
                "fail-single-line-multi-start"]]
    (util/test-file (str "test/parse/comment/nest/" file))))
