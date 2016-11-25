(ns jank.test.type.struct
  (:refer-clojure :exclude [struct])
  (:require [clojure.test :refer :all]
            [jank.test.type.util :as util]))

(deftest struct
  ; TODO: Fix these
  (doseq [file [;"fail-name-used"
                "fail-multiple-definition"
                "fail-invalid-member-type"
                "fail-members-same-name"
                ;"fail-member-function-redefinition"
                ;"fail-member-function-declaration-incorrect"
                ;"fail-extern-declaration"
                "pass-single-member"
                "pass-multiple-members"
                "pass-extern-type-member"
                "pass-struct-member"
                "pass-recursive"
                "pass-declaration"
                "pass-member-declaration"]]
    (util/test-file (str "test/type/struct/" file))))
