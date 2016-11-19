(ns jank.dev
  (:gen-class)
  (:require [jank.core :as core]))

(defn run
  "Analogous to lein run; start jank with the specified args"
  [& args]
  (use 'jank.dev :reload-all)
  (apply core/-main args))

(defn -main [& args]
  (run args))
