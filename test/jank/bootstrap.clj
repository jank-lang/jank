(ns jank.bootstrap
  (:require [clojure.test :refer :all]
            [jank.parse]
            [jank.type.check :as type]
            [jank.codegen :as codegen]))

(defn slurp-resource [file]
  (slurp (clojure.java.io/resource file)))

(defn- parse [file]
  (jank.parse/parse (slurp-resource file)))

(defn valid-parse? [file]
  (parse file)
  true)

(defn valid-type? [file]
  (type/check (parse file))
  true)

(defn valid-codegen? [file]
  (codegen/codegen (first (type/check (parse file))))
  true)
