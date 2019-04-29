(ns com.jeaye.idiolisp.core
  (:gen-class)
  (:require [com.jeaye.idiolisp
             [parse :as parse]]
            [com.jeaye.idiolisp.parse
             [binding :as parse.binding]]))

(defn -main [& args]
  (binding [parse.binding/*input-file* (first args)
            parse.binding/*input-source* (-> args first slurp)]
    (let [parse-tree (parse/parse parse/prelude)]
      parse-tree)))
