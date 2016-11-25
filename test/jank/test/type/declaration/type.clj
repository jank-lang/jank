(ns jank.test.type.declaration.type
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest declaration-type
  ; TODO
  (doseq [file [;"fail-invalid-generic"
                ;"pass-generic"
                ;"pass-generic-multiple"
                "pass-multiple"
                "pass-normal"]]
    (util/test-file (str "test/type/declaration/type/" file))))
