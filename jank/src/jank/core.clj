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
            {:cells parsed}))))))
