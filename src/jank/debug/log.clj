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
       (if-let [scope (:scope %)]
         (assoc % :scope :elided)
         %)
       %)
    item))

; TODO: Use a macro instead; unary has a label which is the pprint of its form
(defn pprint [arg & args]
  (let [cleaned-arg (clean-scope arg)
        cleaned-args (map clean-scope args)]
    (clojure.pprint/pprint (if (not-empty args)
                             (apply vector cleaned-arg cleaned-args)
                             cleaned-arg))
    arg))
