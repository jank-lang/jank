(ns jank.type.expression
  (:require [jank.type.declaration :as declaration])
  (:use clojure.pprint))

(defmulti realize-type
  "Calculates the type of the expression. All sub-expressions must be
   recursively realized."
  (fn [item scope]
    (first item)))

(defmethod realize-type :lambda-definition [item scope]
  (list "ƒ"
        [:specialization-list
         (into [:specialization-list] (rest (nth item 1)))
         (into [:specialization-list] (rest (nth item 2)))]))

(defmethod realize-type :binding-definition [item scope]
  nil)

(defmethod realize-type :function-call [item scope]
  (let [func-name (get-in item [1 1])
        func (declaration/lookup-binding func-name scope)
        arg-types (map #(realize-type % scope) (rest (rest item)))
        generics (second (:type (second func)))
        expected-types (rest (second generics))
        return-types (rest (nth generics 2))]
    (assert (some? func) (str "unknown function: " func-name))
    (assert (= (apply list arg-types) (apply list expected-types))
            (str "invalid function arguments: " func-name))

    ; TODO: Multiple return types
    (when-not (empty? return-types)
      (first return-types))))

(defmethod realize-type :if-statement [item scope]
  ; TODO: if expressions
  nil)

(defmethod realize-type :list [item scope]
  ; TODO
  nil)

(defmethod realize-type :identifier [item scope]
  (let [ident (second item)
        decl (declaration/lookup-binding ident scope)]
    (assert (some? decl) (str "unknown binding: " ident))
    (:type (second decl))))

; Handles integer, string, etc
(defmethod realize-type :default [item scope]
  (-> item first name symbol str list))
