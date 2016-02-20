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

(defn reduce-spaced-map
  "Maps f over coll and collects the results together in a
   space-separated string"
  [f coll]
  (when (not-empty coll)
    (reduce #(str %1 " " %2) (map f coll))))

(defn serialize-type
  "Takes a type in the [:type [:identifier ...] ...] form and flattens it
   into a string for use with name serialization."
  [type]
  (get-in type [1 1]))

(defn serialize-binding-name
  "Takes a lambda binding definition and updates the name to reflect
   the type signature of the lambda. This is needed to work around the lack of
   overloading in certain targets. Returns the full lambda binding definition."
  [item]
  (let [name (second (second item))
        args (second (nth item 2))
        arg-pairs (partition 2 (rest args))
        serialized-name (if (not-empty arg-pairs)
                          (apply str name "_gen"
                                 (reduce (fn [result pair]
                                           (str result "_"
                                                (-> pair
                                                    second
                                                    serialize-type)))
                                         ""
                                         arg-pairs))
                          (str name "_gen_nullary"))]
    (update-in item [1 1] (fn [_] serialized-name))))

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
