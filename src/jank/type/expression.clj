(ns jank.type.expression
  (:require [jank.type.declaration :as declaration])
  (:use clojure.pprint
        jank.assert))

; XXX: migrated
(defmulti realize-type
  "Calculates the type of the expression. All sub-expressions must be
   recursively realized."
  (fn [item scope]
    (:kind item)))

; XXX: migrated
(defn call-signature
  "Calculates the shortened signature of a given function call."
  [item scope]
  ; The value/name of the function might be a function call which returns a
  ; function or a lambda definition directly; we special case for identifiers
  ; so we can lookup overloads. Otherwise, we use the function directly.
  (let [identifier? (= :identifier (:kind (:name item)))
        func-name (if identifier?
                    (:name (:name item))
                    "anonymous-function")
        overloads (if identifier?
                    (declaration/lookup-overloads func-name scope)
                    [(realize-type (:name item) scope)])
        arg-types (apply list (map #(realize-type % scope) (:arguments item)))]
    (type-assert (some? overloads) (str "unknown function " func-name))
    (type-assert (every? declaration/function? overloads)
                 (str "not a function " func-name))

    ; Test all overloads; matches comes back as a vector of declarations
    ; for the matched functions.
    (let [matches (reduce
                    (fn [matched func]
                      (let [generics (:generics (:value func))
                            expected-types (-> generics :values first :values)]
                        ; TODO: Allow comparison of overload superpositions
                        (if (= arg-types expected-types)
                          (conj matched func)
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

      (let [generics (:generics (:value (first matches)))
            return-types (-> generics :values second :values)]
        (type-assert (not (declaration/auto? (first return-types)))
                     (str "call to function " func-name
                          " before its type is deduced"))
        (first matches)))))

(defmethod realize-type :lambda-definition
  [item scope]
  {:kind :type
   :value {:kind :identifier
           :name "ƒ",
           :generics {:kind :specialization-list
                      :values [{:kind :specialization-list
                                :values (filter #(= :type (:kind %))
                                                (:values (:arguments item)))}
                               {:kind :specialization-list
                                :values (filter #(= :type (:kind %))
                                                (:values (:return item)))}]}}})

(defmethod realize-type :binding-definition
  [item scope]
  (type-assert "binding definitions are not expressions"))

; XXX: migrated
(defmethod realize-type :function-call
  [item scope]
  (let [signature (call-signature item scope)
        return (-> signature :value :generics :values second :values first)]
    return))

; XXX: migrated
(defmethod realize-type :if-expression
  [item scope]
  (type-assert (contains? item :else) "no else statement")
  (let [then-type (realize-type (:then item) scope)
        else-type (realize-type (:else item) scope)]
    (internal-assert (some? then-type) "invalid then type")
    (internal-assert (some? else-type) "invalid else type")
    (internal-assert (= then-type else-type)
                     "incompatible if then/else types")
    then-type))

(defmethod realize-type :list
  [item scope]
  ; TODO
  (not-yet-implemented type-assert false "list type realization"))

; XXX: migrated
(defmethod realize-type :identifier
  [item scope]
  (let [ident (:name item)
        decl (declaration/lookup ident scope)]
    (type-assert (some? decl) (str "unknown binding " ident))

    (let [first-decl (first (second decl))]
      (if (declaration/function? first-decl)
        ; Function identifiers yield a superposition of all possible overloads
        (realize-type (assoc item :kind :function-identifier) scope)
        first-decl))))

; XXX: migrated
(defmethod realize-type :function-identifier
  [item scope]
  ; TODO: Wrap in generic variadic lambda
  (not-yet-implemented type-assert false "function identifiers"))

; XXX: migrated
(defmethod realize-type :return
  [item scope]
  ; Realize that which is being returned
  (realize-type (:value item) scope))

; Handles integer, string, etc
; XXX: migrated
(defmethod realize-type :default
  [item scope]
  {:kind :type
   :value {:kind :identifier
           :name (-> item :kind name symbol str)}})

; Empty bodies will realize to nil
(defmethod realize-type nil
  [item scope]
  nil)
