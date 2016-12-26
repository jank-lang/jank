(ns jank.dev
  (:gen-class)
  (:require [jank.core :as core]
            [jank.parse :as parse]
            [jank.test.bootstrap :as bootstrap :refer :all]))

(defn reload []
  (use 'jank.dev :reload-all))

(defn run
  "Analogous to lein run; start jank with the specified args"
  [& args]
  (reload)
  (apply core/-main args))

(defn parses
  ([file] (parses file {}))
  ([file & args]
   (reload)
   (apply parse/parses (slurp file) args)))

(defn -main [& args]
  (apply run args))
