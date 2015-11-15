(ns jank.core
  (:gen-class)
  (:require [jank.parse :as parse]
            [jank.codegen :as codegen]
            [jank.type.check :as type]))

(defn -main
  [& args]
  (-> {:cells (parse/parse (slurp (first args)))}
      type/check
      first
      codegen/codegen))
