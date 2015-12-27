(ns jank.codegen.util
  (:use clojure.pprint))

(defn swap-params [params]
  "Takes the input (i integer b boolean) and gives the C-like
   representation: ((integer i) (boolean b))"
  (map reverse (partition 2 params)))

(defn comma-separate-params [pairs]
  "Turns ((integer i) (boolean b)) into a string like
   \"integer i, boolean b\""
  (clojure.string/join ","
                       (map #(str (first %) " " (second %)) pairs)))

(defn comma-separate-args [args]
  "Turns (foo bar spam) into a string like
   \"foo, bar, spam\""
  (clojure.string/join "," args))

(defn reduce-spaced-map [f coll]
  "Maps f over coll and collects the results together in a
   space-separated string"
  (when (not-empty coll)
    (reduce #(str %1 " " %2) (map f coll))))

(defn end-statement [statement]
  "Ends a statement with a semi-colon. Empty statements are unchanged."
  (if (not-empty statement)
    (str statement ";")
    statement))

(defn print-statement [statement]
  "Prints the statement to stdout, followed by a new line.
   Empty statements are ignored."
  (when-not (empty? statement)
    (println statement)))

(defn partition-definitions [cells]
  "Partitions unsorted cells and returns two sequences.
   The first sequence contains the top-level definitions; the second
   contains all top-level expressions."
  ((juxt filter remove) #(and (= :binding-definition (first %))
                              (= (first (nth % 2)) :lambda-definition))
   cells))
