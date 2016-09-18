(ns jank.debug.log
  (:require [clojure.walk :refer [postwalk]]
            clojure.pprint)
  (:use jank.assert))

(defn clean-scope
  "Removes all :scope values recursively. This makes it
   much easier to print AST nodes."
  [item]
  (postwalk
    #(if (map? %)
       (dissoc % :scope)
       %)
    item))

(defn pprint [item]
  (clojure.pprint/pprint item)
  item)
