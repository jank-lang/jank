(ns com.jeaye.jank.test.parse.util
  (:require [clojure.test :refer [is]]
            [com.jeaye.jank.test.bootstrap :as bootstrap]))

(def error #"parse error")

(defn test-files [path excludes]
  #_(println "[parse] Gathering files...")
  (let [files (bootstrap/files path excludes)]
    (doseq [file-info files]
      #_(if (:skip? file-info)
        (println "[parse] SKIP" (:resource file-info))
        (println "[parse] test" (:resource file-info)))
      (cond
        (:skip? file-info) nil

        (bootstrap/should-fail? file-info)
        (is (thrown-with-msg? clojure.lang.ExceptionInfo
                              error
                              (bootstrap/valid-parse? file-info)))

        :else
        (is (bootstrap/valid-parse? file-info))))))
