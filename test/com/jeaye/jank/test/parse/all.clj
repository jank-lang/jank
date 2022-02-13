(ns com.jeaye.jank.test.parse.all
  (:require [clojure.test :refer [deftest use-fixtures]]
            [com.jeaye.jank.test.bootstrap :as bootstrap]))

(use-fixtures :once bootstrap/with-instrumentation)

(deftest all
  (bootstrap/test-files "[parse]"
                        "parse error"
                        bootstrap/valid-parse?
                        "dev/resources/test/neo-parse/"
                        [#".*/keyword/pass-qualified\.jank"
                         #".*/keyword/pass-aliased\.jank"]))
