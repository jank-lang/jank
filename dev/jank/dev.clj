(ns jank.dev
  (:gen-class)
  (:require [com.jeaye.jank.core :as core]
            [com.jeaye.jank.parse :as parse]
            [com.jeaye.jank.test.bootstrap :as bootstrap]))

(defn reload! []
  (use 'jank.dev :reload-all))

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
