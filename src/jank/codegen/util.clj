(ns jank.codegen.util
  (:use clojure.pprint))

(defn swap-params
  "Takes the input (i integer b boolean) and gives the C-like
   representation: ((integer i) (boolean b))"
  [params]
  (map reverse (partition 2 params)))

(defn comma-separate-params
  "Turns ((integer i) (boolean b)) into a string like
   \"integer i, boolean b\""
  [pairs]
  (clojure.string/join ","
                       (map #(str (first %) " " (second %)) pairs)))

(defn comma-separate-args
  "Turns (foo bar spam) into a string like
   \"foo, bar, spam\""
  [args]
  (clojure.string/join "," args))

(defn reduce-spaced-map [f coll]
  "Maps f over coll and collects the results together in a
   space-separated string"
  (when (not-empty coll)
    (reduce #(str %1 " " %2) (map f coll))))

(defn end-statement
  "Ends a statement with a semi-colon. Empty statements are unchanged."
  [statement]
  (if (not-empty statement)
    (str statement ";")
    statement))

(defn print-statement
  "Prints the statement to stdout, followed by a new line.
   Empty statements are ignored."
  [statement]
  (when-not (empty? statement)
    (println statement)))
