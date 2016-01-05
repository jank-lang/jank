(ns jank.type.expression
  (:require [jank.type.declaration :as declaration])
  (:use clojure.pprint
        jank.assert))

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
  (type-assert "binding definitions are not expressions"))

(defmethod realize-type :function-call [item scope]
  (let [func-name (get-in item [1 1])
        overloads (declaration/lookup-overloads func-name scope)
        arg-types (apply list
                         (declaration/shorten-types
                           (map #(realize-type % scope) (rest (rest item)))))]
    (type-assert (some? overloads) (str "unknown function " func-name))

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
      (type-assert (not-empty matches)
                   (str "no matching function call to " func-name
                        " with argument types " arg-types))
      (type-assert (= 1 (count matches))
                   (str "ambiguous function call to " func-name
                        " with argument types " arg-types))
      ; TODO: Test this
      (type-assert (not= '(("auto")) (first matches))
                   (str "call to function " func-name
                        " before its type is deduced"))

      (when-not (empty? (first matches))
        (ffirst matches)))))

(defmethod realize-type :if-expression [item scope]
  (type-assert (= 4 (count item)) "no else statement")
  (let [then-type (realize-type (second (nth item 2)) scope)
        else-type (realize-type (second (nth item 3)) scope)]
    (internal-assert (= then-type else-type)
                     "incompatible if then/else types")
    then-type))

(defmethod realize-type :list [item scope]
  ; TODO
  (not-yet-implemented type-assert false "list type realization"))

(defmethod realize-type :identifier [item scope]
  (let [ident (second item)
        decl (declaration/lookup-binding ident scope)]
    (type-assert (some? decl) (str "unknown binding " ident))
    (:type (get-in decl [1 0]))))

(defmethod realize-type :return [item scope]
  ; Realize that which is being returned
  (realize-type (second item) scope))

; Handles integer, string, etc
(defmethod realize-type :default [item scope]
  (-> item first name symbol str list))

; Empty bodies will realize to nil
(defmethod realize-type nil [item scope]
  nil)
