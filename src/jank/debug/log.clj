(ns jank.debug.log
  (:require [clojure.walk :refer [postwalk]])
  (:use clojure.pprint
        jank.assert))

(defn clean-scope
  [item]
  (postwalk
    #(if (map? %)
       (dissoc % :scope)
       %)
    item))
