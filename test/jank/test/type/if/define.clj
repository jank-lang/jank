(ns jank.test.type.if.define
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest if-define
  (doseq [file ["fail-integer-condition"
                "pass-boolean-condition"
                "pass-with-else"]]
    (util/test-file (str "test/type/if/define/" file))))
