(ns com.jeaye.jank.dev
  (:require [clojure.spec.alpha :as s]
            [orchestra.core :refer [defn-spec]]
            [orchestra.spec.test :as stest]
            [expound.alpha :as expound]
            [com.jeaye.jank.core :as core]
            [com.jeaye.jank.parse :as parse]
            [com.jeaye.jank.test.bootstrap :as bootstrap]))

(defn reload! []
  (use 'com.jeaye.jank.dev :reload-all)
  (s/check-asserts true)
  (stest/instrument)
  (->> (constantly (expound/custom-printer {:show-valid-values? true}))
       (alter-var-root #'s/*explain-out*))
  nil)

(defmacro def-reload
  [def-name to-wrap]
  `(defn ~def-name [& args#]
     (reload!)
     (apply ~to-wrap args#)))

(def-reload parse bootstrap/try-parse)

(defn run
  "Analogous to lein run; start jank with the specified args"
  [& args]
  (reload!)
  (apply core/-main args))

(defn parses
  ([file] (parses file {}))
  ([file & args]
   (reload!)
   (apply parse/parses (slurp file) args)))

(defn -main [& args]
  (apply run args))
