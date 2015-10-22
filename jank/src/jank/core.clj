(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def parse
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn map-from [index func coll]
  (vec (concat (take index coll)
               (map func (drop index coll)))))

(defmulti handle
  (fn [current ast]
    (first current)))

(defmethod handle :lambda-definition [current ast]
  ; (λ (args) (rets) ...)
  (map-from 3 #(handle %1 ast) current))

(defmethod handle :macro-definition [current ast]
  ; (macro name (types) (args) ...)
  (map-from 4 #(handle %1 ast) current))

(defmethod handle :binding-definition [current ast]
  ; (bind name value)
  (map-from 2 #(handle %1 ast) current))

(defmethod handle :function-call [current ast]
  ; (foo ...)
  (map-from 1 #(handle %1 ast) current))

(defmethod handle :default [current ast]
  current)

(defn swap-params [params]
  "Takes the input (i integer b boolean) and gives the C-like
   representation: ((integer i) (boolean b))"
  (map reverse (partition 2 params)))

(defn comma-separate-params [pairs]
  "Turns ((integer i) (boolean b)) into a string like
   \"integer i, boolean b\""
  (clojure.string/join ", "
                       (map #(str (first %1) " " (second %1)) pairs)))

(defn comma-separate-args [args]
  "Turns (foo bar spam) into a string like
   \"foo, bar, spam\""
  (clojure.string/join ", " args))

(defn reduce-spaced-map [f coll]
  "Maps f over coll and collects the results together in a
   space-separated string"
  (reduce #(str %1 " " %2) (map f coll)))

(defmulti codegen-impl
  (fn [current]
    (first current)))

(defmethod codegen-impl :lambda-definition [current]
  (str "[&]"
       (codegen-impl (second current)) ; Params
       " -> "
       (codegen-impl (second (nth current 2))) ; Return
       " { "
       (reduce-spaced-map codegen-impl (drop 3 current)) ; Body
       " }"))

(defmethod codegen-impl :binding-definition [current]
  "")

(defmethod codegen-impl :function-call [current]
  (str (codegen-impl (second current)) ; Name
       "("
       (comma-separate-args (map codegen-impl (drop 2 current)))
       ");")) ; Args

(defmethod codegen-impl :argument-list [current]
  (str "("
       (comma-separate-params
         (swap-params
           (map codegen-impl (rest (second current)))))
       ")"))

(defmethod codegen-impl :list [current]
  (str "("
       (reduce-spaced-map codegen-impl (rest current))
       ")"))

(defmethod codegen-impl :string [current]
  (str "\"" (second current) "\""))

(defmethod codegen-impl :identifier [current]
  ; Sanitize
  (second current))

(defmethod codegen-impl :default [current]
  "")

(defn codegen [ast]
  (doseq [current (:cells ast)]
    (println (codegen-impl current))))

(defn -main
  [& args]
  (codegen
    (let [parsed (parse (slurp (first args)))]
      (when parsed
        (loop [current (first parsed)
               remaining (rest parsed)
               ast {:cells []}]
          (println current)
          (cond
            (nil? current) ast
            :else (recur (first remaining)
                         (rest remaining)
                         (update ast :cells conj (handle current ast)))))))))
