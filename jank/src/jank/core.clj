(ns jank.core
  (:gen-class)
  (:require [jank.parse :as parse :refer [parse]]
            [jank.codegen :as codegen :refer [codegen]]
            [jank.type.check :as check :refer [check]]))

(defn -main
  [& args]
  (-> {:cells (parse/parse (slurp (first args)))}
      check/check
      first
      codegen/codegen))
