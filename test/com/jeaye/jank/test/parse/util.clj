(ns com.jeaye.jank.test.parse.util
  (:require [clojure.test :refer [is]]
            [com.jeaye.jank.test.bootstrap :as bootstrap]))

(def error #"parse error")

(defn test-files [path excludes]
  (println "[parse] Gathering files...")
  (let [files (bootstrap/files path excludes)]
    (doseq [file files]
      (println "[parse] testing" file)
      (if (bootstrap/should-fail? file)
        (is (thrown-with-msg? clojure.lang.ExceptionInfo
                              error
                              (bootstrap/valid-parse? file)))
        (is (bootstrap/valid-parse? file))))))
