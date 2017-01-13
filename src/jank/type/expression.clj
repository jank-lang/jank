(ns jank.type.expression
  (:require [jank.parse.fabricate :as fabricate]
            [jank.type.scope.type-declaration :as type-declaration]
            [jank.type.scope.binding-declaration :as binding-declaration])
  (:use jank.debug.log
        jank.assert))

(defmulti realize-type
  "Calculates the type of the expression. All sub-expressions must be
   recursively realized."
  (fn [item scope]
    (:kind item)))

(defn overload-args
  "Extracts just a sequence of all overload args. This is used for cleaner
   error messages."
  [overloads]
  (apply vector
         (map #(-> %
                   :value
                   :generics
                   :values
                   first
                   :values)
              overloads)))

(defn function-name
  "Extracts the name identifier out of function calls and provides a constant
   in the case of anonymous functions."
  [item identifier?]
  (if identifier?
    (:name (:name item))
    "anonymous-function"))

(defn overload-matches
  "Finds all matching overloads for a call. The matching overloads may be generic
   and may need instantiation."
  [item scope]
  ; The value/name of the function might be a function call which returns a
  ; function or a lambda definition directly; we special case for identifiers
  ; so we can lookup overloads. Otherwise, we use the function directly.
  (let [identifier? (= :identifier (:kind (:name item)))
        func-name (function-name item identifier?)
        overloads (if identifier?
                    (binding-declaration/lookup-overloads func-name scope)
                    [(realize-type (:name item) scope)])
        arg-types (apply list (map #(realize-type % scope) (:arguments item)))]
    (type-assert (some? overloads) (str "unknown function " func-name))
    (type-assert (every? type-declaration/function? overloads)
                 (str "not a function " func-name))

    ; TODO: Handle generic calls; match generics explicitly
    ; Test all overloads
    (let [stripped-arg-types (map type-declaration/strip arg-types)
          match-info (map
                       ; TODO: Move this into its own function
                       #(let [type-generics (-> % :value :generics)
                              expected-types (-> type-generics
                                                 :values first :values)
                              stripped-expected (map type-declaration/strip
                                                     expected-types)
                              pairs (map vector
                                         stripped-arg-types
                                         stripped-expected)
                              generics (into #{} (-> % :generics :values))]
                          ; TODO: Allow comparison of overload superpositions
                          (cond
                            (not= (count stripped-arg-types) (count stripped-expected))
                            nil

                            (every? (fn [[l r]] (= l r)) pairs)
                            [% :full]

                            (every? (fn [[arg expected]]
                                      ; TODO: Allow expected to be auto for shorthand generics
                                      (or (= arg expected)
                                          (generics expected)))
                                    pairs)
                            [% :partial]

                            :else
                            nil))
                       overloads)
          matches (filter some? match-info)
          full-matches (filter #(= :full (second %)) matches)
          partial-matches (filter #(= :partial (second %)) matches)]
      (type-assert (not-empty matches)
                   (str "no matching function call to " func-name
                        " with argument types " arg-types
                        " expected one of " (overload-args overloads)))
      (type-assert (and (>= 1 (count full-matches))
                        (>= 1 (count partial-matches)))
                   (str "ambiguous function call to " func-name
                        " with argument types " arg-types
                        " expected one of " (overload-args overloads)))
      {:full-matches full-matches
       :partial-matches partial-matches
       :identifier? identifier?
       :function-name func-name})))

(defn call-signature
  "Calculates the signature of a given function call."
  [item scope]
  (let [matches (overload-matches item scope)
        match (ffirst (:full-matches matches))
        generics (:generics (:value match))
        return-types (-> generics :values second :values)]
    (type-assert (not (type-declaration/auto? (first return-types)))
                 (str "call to function " (:function-name matches)
                      " before its type is deduced"))
    match))

(defmethod realize-type :lambda-definition
  [item scope]
  (let [ret (fabricate/function-type (filter #(= :type (:kind %))
                                             (:values (:arguments item)))
                                     (filter #(= :type (:kind %))
                                             (:values (:return item))))]
    (if (:generic? item)
      (assoc ret :generics (:generics item))
      ret)))

(defmethod realize-type :binding-definition
  [item scope]
  (type-assert "binding definitions are not expressions"))

(defmethod realize-type :macro-function-call
  [item scope]
  (realize-type (assoc item :kind :function-call) scope))

(defmethod realize-type :syntax-definition
  [item scope]
  (fabricate/type "syntax"))

(defmethod realize-type :function-call
  [item scope]
  (let [signature (call-signature item scope)
        return (-> signature :value :generics :values second :values first)]
    return))

(defmethod realize-type :if-expression
  [item scope]
  (type-assert (contains? item :else) "no else statement")
  (let [then-type (realize-type (:value (:then item)) scope)
        else-type (realize-type (:value (:else item)) scope)]
    (internal-assert (some? then-type) "invalid then type")
    (internal-assert (some? else-type) "invalid else type")
    (internal-assert (= (type-declaration/strip then-type)
                        (type-declaration/strip else-type))
                     (str "incompatible if then/else types "
                          then-type
                          " and "
                          else-type))
    then-type))

(defmethod realize-type :identifier
  [item scope]
  (let [ident (:name item)
        decl (binding-declaration/lookup ident scope)]
    (type-assert (some? decl) (str "unknown binding " ident))

    (let [first-decl (first (second decl))]
      (if (type-declaration/function? first-decl)
        ; Function identifiers yield a superposition of all possible overloads
        (realize-type (assoc item :kind :function-identifier) scope)
        first-decl))))

(defmethod realize-type :function-identifier
  [item scope]
  ; TODO: Wrap in generic variadic lambda
  (not-yet-implemented type-assert false "function identifiers"))

(defmethod realize-type :return
  [item scope]
  ; Realize that which is being returned
  (realize-type (:value item) scope))

(defmethod realize-type :new-expression
  [item scope]
  (-> item
      :specialization-list
      :values
      first))

; Handles integer, string, etc
(defmethod realize-type :default
  [item scope]
  (fabricate/type (-> item :kind name symbol str)))

; Empty bodies will realize to nil
(defmethod realize-type nil
  [item scope]
  nil)
