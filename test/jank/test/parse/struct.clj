(ns jank.test.parse.struct
  (:refer-clojure :exclude [struct])
  (:require [clojure.test :refer :all]
            [jank.test.parse.all :as util]))

(deftest struct
  (doseq [file ["fail-no-members"
                "fail-empty-member"
                "fail-member-too-many-fields"
                "fail-no-name"
                "fail-no-name-some-members"
                "fail-too-many-names"
                "fail-non-list-member"
                "fail-multiple-members-all-values"
                "fail-multiple-members-mixed-values"
                "pass-one-member-no-value"
                "pass-unicode-name"
                "pass-multiple-members-no-values"]]
    (util/test-file (str "test/parse/struct/" file))))
