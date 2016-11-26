(ns jank.test.parse.comment.whitespace
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest comment-whitespace
  (doseq [file ["pass-lambda"]]
    (util/test-file (str "test/parse/comment/whitespace/" file))))
