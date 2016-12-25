(ns jank.test.parse.string
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

; TODO: Refactor into escape and normal tests
(deftest string
  (doseq [file ["escape/pass-lots-of-unescaped-closes"
                "escape/pass-unescaped-both"
                "escape/pass-unescaped-close"
                "escape/pass-unescaped-open"
                "escape/pass-escape-both"
                "escape/pass-escape-close"
                "escape/pass-escape-open"
                "pass-numbers"
                "pass-nested-quotes"
                "fail-unmatched-odd.jank"
                "fail-unmatched.jank"
                "pass-multi-line.jank"
                "pass-unicode.jank"]]
    (util/test-file (str "test/parse/string/" file))))
