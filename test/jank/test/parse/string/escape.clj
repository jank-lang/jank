(ns jank.test.parse.string.escape
  (:require [clojure.test :refer :all]
            [jank.test.parse.util :as util]))

(deftest string-escape
  (doseq [file ["escape/pass-lots-of-unescaped-closes"
                "escape/pass-unescaped-both"
                "escape/pass-unescaped-close"
                "escape/pass-unescaped-open"
                "escape/pass-escape-both"
                "escape/pass-escape-close"
                "escape/pass-escape-open"]]
    (util/test-file (str "test/parse/string/escape/" file))))
