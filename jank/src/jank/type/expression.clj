(ns jank.type.expression
  (:require [jank.type.declaration :as declaration :refer [lookup-binding]]))

(defmulti realize-type
  "Calculates the type of the expression. All sub-expressions must be
   recursively realized."
  (fn [item scope]
    (first item)))

(defmethod realize-type :lambda-definition [item scope]
  ; TODO: function type
  nil)

(defmethod realize-type :binding-definition [item scope]
  nil)

(defmethod realize-type :function-call [item scope]
  ; TODO: Recursively handle args
  (let [func-name (get-in item [1 1])
        func (declaration/lookup-binding func-name scope)
        arg-types (map #(realize-type % scope) (rest (rest item)))]
    (assert (some? func) (str "Unknown function: " func-name))
    (apply println "!!" arg-types)
    nil))

(defmethod realize-type :if-statement [item scope]
  ; TODO: if expressions
  nil)

(defmethod realize-type :list [item scope]
  ; TODO
  nil)

(defmethod realize-type :identifier [item scope]
  (let [ident (second item)
        decl (declaration/lookup-binding ident scope)]
    (assert (some? decl) (str "Unknown binding: " ident))
    (:type (second decl))))

; Handles integer, string, etc
(defmethod realize-type :default [item scope]
  ; Lists are used to allow for specializations in more complex types
  (-> item first name symbol list str))
