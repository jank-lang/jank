(ns jank.parse-test
  (:require [clojure.test :refer :all]
            [jank.bootstrap :refer :all]))

(deftest parse
  (is (valid-parse? "test/parse/paren/match/fail_close_nothing_else.jank"))
  (is (valid-parse? "test/parse/ident/ascii/pass_good_chars.jank")))
