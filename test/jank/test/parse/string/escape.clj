(ns jank.test.parse.string.escape
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest string-escape
  (doseq [file ["pass-lots-of-unescaped-closes"
                "pass-unescaped-both"
                "pass-unescaped-close"
                "pass-unescaped-open"
                "pass-escape-both"
                "pass-escape-close"
                "pass-escape-open"]]
    (util/test-file (str "test/parse/string/escape/" file))))
