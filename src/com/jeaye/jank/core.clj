(ns com.jeaye.jank.core
  (:gen-class)
  (:require [com.jeaye.jank
             [parse :as parse]]))

(defn -main
  [& args]
  (let [parse-info (parse/parse parse/prelude
                                (first args)
                                (-> args first slurp))]
    (::parse/tree parse-info)))
