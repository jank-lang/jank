(ns jank.type.expression
  (:require [jank.type.declaration :as declaration])
  (:use clojure.pprint))

(defmulti realize-type
  "Calculates the type of the expression. All sub-expressions must be
   recursively realized."
  (fn [item scope]
    (first item)))

(defmethod realize-type :lambda-definition [item scope]
  (letfn [(remove-identifiers [item]
            (filter #(not= :identifier (first %)) item))]
    (list "ƒ"
      [:specialization-list
       (into [:specialization-list] (remove-identifiers (rest (nth item 1))))
       (into [:specialization-list] (remove-identifiers (rest (nth item 2))))])))

(defmethod realize-type :binding-definition [item scope]
  nil)

(defmethod realize-type :function-call [item scope]
  (let [func-name (get-in item [1 1])
        overloads (second (declaration/lookup-binding func-name scope))
        arg-types (apply list (map #(realize-type % scope) (rest (rest item))))]
    (assert (some? overloads) (str "unknown function: " func-name))

    ; Test all overloads; matches comes back as a vector of the return types
    ; for the matched functions.
    (let [matches (reduce
                    (fn [matched func]
                      (let [generics (second (:type func))
                            expected-types (apply list (rest (second generics)))
                            return-types (rest (nth generics 2))]
                        (if (= arg-types expected-types)
                          (conj matched return-types)
                          matched)))
                    []
                    overloads)]
      (assert (not-empty matches)
              (str "no matching function call to: " func-name
                   " with argument types: " arg-types))
      (assert (= 1 (count matches))
              (str "ambiguous function call to: " func-name
                   " with argument types: " arg-types))

      ; TODO: Multiple return types
      (pprint matches)
      (when-not (empty? (first matches))
        (ffirst matches)))))

(defmethod realize-type :if-statement [item scope]
  ; TODO: if expressions
  ; Realizing the type of an if expression means that we need
  ; it for a value; in this case, we require both then and else clauses
  ; and they must evaluate to the same type.
  nil)

(defmethod realize-type :list [item scope]
  ; TODO
  nil)

(defmethod realize-type :identifier [item scope]
  (let [ident (second item)
        decl (declaration/lookup-binding ident scope)]
    (assert (some? decl) (str "unknown binding: " ident))
    (:type (get-in decl [1 0]))))

; Handles integer, string, etc
(defmethod realize-type :default [item scope]
  (-> item first name symbol str list))
