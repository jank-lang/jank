(ns jank.core
  (:gen-class)
  (:require [jank.parse :as parse :refer [parse]]
            [jank.codegen :as codegen :refer [codegen]]
            [jank.type.check :as check :refer [check]]))

(defn -main
  [& args]
  (codegen/codegen
    (first
      (check/check
        (let [parsed (parse/parse (slurp (first args)))]
          (when parsed
            (loop [current (first parsed)
                   remaining (rest parsed)
                   ast {:cells []}]
              (cond
                (nil? current) ast
                :else (recur (first remaining)
                             (rest remaining)
                             (update ast
                                     :cells
                                     conj
                                     (parse/handle current ast)))))))))))
