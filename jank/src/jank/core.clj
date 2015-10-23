(ns jank.core
  (:gen-class)
  (:require [jank.parse :as parse :refer [parse]]
            [jank.codegen :as codegen :refer [codegen]]))

(defn -main
  [& args]
  (codegen/codegen
    (let [parsed (parse/parse (slurp (first args)))]
      (when parsed
        (loop [current (first parsed)
               remaining (rest parsed)
               ast {:cells []}]
          (println current)
          (cond
            (nil? current) ast
            :else (recur (first remaining)
                         (rest remaining)
                         (update ast :cells conj (parse/handle current ast)))))))))
