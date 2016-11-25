(ns jank.test.type.lambda.superposition
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest superpositions
  ; TODO
  (doseq [file [;"pass-lazy"
                ;"pass-outer-and-inner"
                ;"pass-outer"
                ;"pass-parameter"
                ]]
    (util/test-file (str "test/type/lambda/superposition/" file))))
