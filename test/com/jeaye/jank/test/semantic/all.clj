(ns com.jeaye.jank.test.semantic.all
  (:require [clojure.test :refer [deftest use-fixtures]]
            [com.jeaye.jank.test.bootstrap :as bootstrap]))

(use-fixtures :once bootstrap/with-instrumentation)

(deftest all
  (bootstrap/test-files "[semantic]"
                        "type error"
                        bootstrap/valid-semantic-check?
                        "dev/resources/test/semantic/"
                        []))
