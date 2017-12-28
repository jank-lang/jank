(ns com.jeaye.jank.core
  (:gen-class)
  (:require [com.jeaye.jank
             [parse :as parse]]
            [com.jeaye.jank.parse
             [binding :as parse.binding]]))

(defn -main
  [& args]
  (binding [parse.binding/*input-file* (first args)
            parse.binding/*input-source* (-> args first slurp)]
    (let [parse-info (parse/parse parse/prelude)]
      (::parse/tree parse-info))))
