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
    (declaration/shorten-types
      (list "ƒ"
            [:specialization-list
             (into [:specialization-list]
                   (remove-identifiers (rest (nth item 1))))
             (into [:specialization-list]
                   (remove-identifiers (rest (nth item 2))))]))))

(defmethod realize-type :binding-definition [item scope]
  (type-assert "binding definitions are not expressions"))

(defmethod realize-type :function-call [item scope]
  ; The value/name of the function might be a function call which returns a
  ; function or a lambda definition directly; we special case for identifiers
  ; so we can lookup overloads. Otherwise, we use the function directly.
  (let [identifier? (= :identifier (get-in item [1 0]))
        func-name (if identifier?
                    (get-in item [1 1])
                    "anonymous-function")
        overloads (if identifier?
                    (declaration/lookup-overloads func-name scope)
                    [{:type (realize-type (nth item 1) scope)}])
        arg-types (apply list (map #(realize-type % scope) (rest (rest item))))]
    (type-assert (some? overloads) (str "unknown function " func-name))
    (type-assert (every? (comp declaration/function? :type) overloads)
                 (str "not a function " func-name))

    ; Test all overloads; matches comes back as a vector of the return types
    ; for the matched functions.
    (let [matches (reduce
                    (fn [matched func]
                      (let [generics (second (:type func))
                            expected-types (apply list (rest (second generics)))
                            return-types (rest (nth generics 2))]
                        ; TODO: Allow comparison of overload superpositions
                        (if (= arg-types expected-types)
                          (conj matched return-types)
                          matched)))
                    []
                    overloads)]
      (type-assert (not-empty matches)
                   (str "no matching function call to " func-name
                        " with argument types " arg-types
                        " expected one of " overloads))
      (type-assert (= 1 (count matches))
                   (str "ambiguous function call to " func-name
                        " with argument types " arg-types
                        " expected one of " overloads))
      (type-assert (not (declaration/auto? (ffirst matches)))
                   (str "call to function " func-name
                        " before its type is deduced"))

      (when-not (empty? (first matches))
        (declaration/shorten-types
          (ffirst matches))))))

(defmethod realize-type :if-expression [item scope]
  (type-assert (some #(and (vector? %) (= (first %) :else)) item)
               "no else statement")
  (let [then-type (realize-type (second (nth item 2)) scope)
        else-type (realize-type (second (nth item 3)) scope)]
    (internal-assert (= then-type else-type)
                     "incompatible if then/else types")
    (declaration/shorten-types then-type)))

(defmethod realize-type :list [item scope]
  ; TODO
  (not-yet-implemented type-assert false "list type realization"))

(defmethod realize-type :identifier [item scope]
  (let [ident (second item)
        decl (declaration/lookup ident scope)]
    (type-assert (some? decl) (str "unknown binding " ident))

    ; Function identifiers yield a superposition of all possible overloads
    (let [first-decl (declaration/shorten-types (:type (first (nth decl 1))))]
      (if (declaration/function? first-decl)
        (realize-type (update-in item [0] (fn [_] :function-identifier)) scope)
        first-decl))))

(defmethod realize-type :function-identifier [item scope]
  ; TODO: Wrap in generic variadic lambda
  (not-yet-implemented type-assert false "function identifiers"))

(defmethod realize-type :return [item scope]
  ; Realize that which is being returned
  (realize-type (second item) scope))

; Handles integer, string, etc
(defmethod realize-type :default [item scope]
  (declaration/shorten-types (-> item first name symbol str list)))

; Empty bodies will realize to nil
(defmethod realize-type nil [item scope]
  nil)
