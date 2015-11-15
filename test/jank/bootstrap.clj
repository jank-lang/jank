(ns jank.bootstrap
  (:require [jank.parse :as parse]
            [jank.type.check :as type]
            [jank.codegen :as codegen]))

(defn slurp-resource [file]
  (slurp (clojure.java.io/resource file)))

(defn valid-parse? [file]
  (parse/parse (slurp-resource file))
  true)

(defn valid-type? [file]
  (type/check (parse/parse (slurp-resource file)))
  true)

(defn valid-codegen? [file]
  (codegen/codegen (first (type/check (parse/parse (slurp-resource file)))))
  true)
