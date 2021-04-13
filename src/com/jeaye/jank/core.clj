(ns com.jeaye.jank.core
  (:gen-class)
  (:require [com.jeaye.jank.parse :as parse]
            [com.jeaye.jank.parse.binding :as parse.binding]
            [com.jeaye.jank.codegen :as codegen]))

(defn parse+codegen [file]
  (binding [parse.binding/*input-file* file
            parse.binding/*input-source* (slurp file)]
    (let [parse-tree (parse/parse parse/prelude)
          code (codegen/generate parse-tree)]
      code)))

(defn -main [& args]
  (println (parse+codegen (first args)))
  (shutdown-agents))

(comment
  (parse+codegen "ray.jank"))
