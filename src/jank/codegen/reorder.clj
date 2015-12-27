(ns jank.codegen.reorder
  (:use clojure.pprint))

; TODO: Rename?
(defn reorder [cells]
  "Partitions unsorted cells and returns two sequences.
   The first sequence contains the top-level definitions; the second
   contains all top-level expressions."
  ((juxt filter remove) #(and (= :binding-definition (first %))
                              (= (first (nth % 2)) :lambda-definition))
   cells))
