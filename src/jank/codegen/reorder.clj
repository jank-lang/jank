(ns jank.codegen.reorder
  (:use clojure.pprint))

(defn reorder [cells]
  "Performs a stable sort on all cells, moving function definitions before
   all other statements and expressions."
  (sort
    (fn [a b]
      (and (and (= :binding-definition (first a))
                (= (first (nth a 2)) :lambda-definition))
           (not= :binding-definition (first b))))
    cells))
