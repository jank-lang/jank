(ns com.jeaye.jank.core
  (:gen-class)
  (:require [com.jeaye.jank
             [parse :as parse]]))

(defn -main
  [& args]
  (parse/parse parse/prelude (-> args first slurp)))
