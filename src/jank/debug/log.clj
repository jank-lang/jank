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

(defn pprint [arg & args]
  (let [cleaned-arg (clean-scope arg)
        cleaned-args (map clean-scope args)]
    (clojure.pprint/pprint (if (not-empty args)
                             (apply vector cleaned-arg cleaned-args)
                             cleaned-arg))
    arg))
