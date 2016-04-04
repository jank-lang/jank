(ns jank.codegen.util
  (:require [jank.type.declaration :as declaration]
            [jank.codegen.sanitize :as sanitize])
  (:use jank.assert
        jank.debug.log
        clojure.pprint))

; XXX: migrated
(defn swap-params
  "Takes the input (i integer b boolean) and gives the C-like
   representation: ((integer i) (boolean b))"
  [params]
  (map reverse (partition 2 params)))

; XXX: migrated
(defn comma-separate-params
  "Turns ((integer i) (boolean b)) into a string like
   \"integer i, boolean b\""
  [pairs]
  (clojure.string/join ","
                       (map #(str (first %) " " (second %)) pairs)))

; XXX: migrated
(defn comma-separate-args
  "Turns (foo bar spam) into a string like
   \"foo, bar, spam\""
  [args]
  (clojure.string/join "," args))

; XXX: migrated
(defn reduce-spaced-map
  "Maps f over coll and collects the results together in a
   delim-separated string. The delim defaults to a space."
  ([f coll]
   (reduce-spaced-map f coll " "))
  ([f coll delim]
   (when (not-empty coll)
     (reduce #(str %1 delim %2) (map f coll)))))

; XXX: migrated
(defmulti mangle
  "Flattens the item into a string for use with name serialization."
  (fn [item]
    (:kind item)))

; TODO: Move to mangle namespace
; TODO: Hash mangled type
(defmethod mangle :type
  [item]
  (sanitize/sanitize-str (mangle (:value item))))

; XXX: migrated
(defmethod mangle :identifier
  [item]
  (sanitize/sanitize-str
    (let [ret (str (:name item) "_t")]
      (if (contains? item :generics)
        (apply str ret (map mangle (-> item :generics :values)))
        ret))))

; XXX: migrated
(defmethod mangle :specialization-list
  [item]
  (sanitize/sanitize-str
    (apply str (map mangle (:values item)))))

; XXX: migrated
(defmethod mangle :default
  [item]
  (codegen-assert false (str "invalid item to mangle " item)))

(defn mangle-binding-name
  "Takes a lambda binding definition and updates the name to reflect
   the type signature of the lambda. This is needed to work around the lack of
   overloading in certain targets. Returns the full lambda binding definition."
  [item]
  (let [name (second (second item))
        args (second (nth item 2))
        arg-pairs (partition 2 (rest args))
        mangled-name (if (not-empty arg-pairs)
                       (apply str name
                              (reduce (fn [result pair]
                                        (str result
                                             (-> pair
                                                 second
                                                 mangle)))
                                      ""
                                      arg-pairs))
                       (str name "_gen_nullary"))]
    (update-in item [1 1] (fn [_] mangled-name))))

; XXX: migrated
(defmethod mangle :function-call
  [item]
  (sanitize/sanitize-str (str (:name (:name item))
                              (mangle (:signature item)))))

; XXX: migrated
(defn end-statement
  "Ends a statement with a semi-colon. Empty statements are unchanged."
  [statement]
  (if (not-empty statement)
    (str statement ";")
    statement))

; XXX: migrated
(defn print-statement
  "Prints the statement to stdout, followed by a new line.
   Empty statements are ignored."
  [statement]
  (when-not (empty? statement)
    (println statement)))
