(ns jank.type.check
  (:require [jank.parse.fabricate :as fabricate]
            [jank.type.scope.type-declaration :as type-declaration]
            [jank.type.scope.type-definition :as type-definition]
            [jank.type.scope.binding-declaration :as binding-declaration]
            [jank.type.scope.binding-definition :as binding-definition]
            [jank.type.scope.macro-definition :as macro-definition]
            [jank.type.scope.util :as scope.util]
            [jank.type.expression :as expression]
            [jank.type.return :as return]
            [jank.interpret.scope.prelude :as interpret.scope.prelude]
            [jank.interpret.macro :as macro]
            [jank.interpret.escape :as escape])
  (:use jank.assert
        jank.debug.log))

(defmulti check-item
  "Type checks the given expression. Returns a cons of the typed
   expression and the updated scope."
  (fn [item scope]
    (let [kind (:kind item)]
      (if (#{:string :integer :boolean :real :identifier} kind)
        :passthrough
        kind))))

(defn check
  "Builds type information on the parsed source. Returns
   a list of the typed source and the top-level scope."
  ([parsed]
   (check parsed (scope.util/new-empty)))
  ([parsed parent-scope]
   ;(pprint (list "parsed:" parsed))
   (loop [item (first (:cells parsed))
          remaining (rest (:cells parsed))
          checked-cells []
          scope parent-scope]
     ;(pprint (list "scope:" scope))
     (if (nil? item)
       (assoc parsed
              :cells checked-cells
              :scope scope)
       (let [checked-item (check-item item scope)]
         (recur (first remaining)
                (rest remaining)
                (conj checked-cells checked-item)
                (:scope checked-item)))))))

(defmethod check-item :binding-declaration
  [item scope]
  (assoc item :scope (binding-declaration/add-to-scope item scope)))

(defmethod check-item :type-declaration
  [item scope]
  (assoc item :scope (type-declaration/add-to-scope item scope)))

(defmethod check-item :lambda-instantiation
  [item scope]
  (let [args (:arguments item)
        return (:return item)
        new-scope (scope.util/new-empty scope)
        checked-args (check-item args new-scope)
        checked-return (check-item return (:scope checked-args))
        checked-body (check {:cells (:body item)} (:scope checked-return))
        updated-item (assoc item
                            :kind :lambda-definition
                            :arguments checked-args ; TODO: Move these back into def?
                            :return checked-return
                            :body (:cells checked-body))
        item-with-return (return/add-explicit-returns updated-item
                                                      (:scope checked-body))]
    (assoc item-with-return
           :scope scope)))

(defmethod check-item :lambda-definition
  [item scope]
  (let [updated-item (assoc item
                            :scope scope)
        ret-item (if (:generic? updated-item)
                   ; TODO: Two-phase name lookup
                   updated-item ; Don't check until instantiation time
                   (check-item (assoc updated-item
                                      :kind :lambda-instantiation)
                               scope))]
    ret-item))

(defmethod check-item :struct-definition
  [item scope]
  (type-assert (nil? (type-definition/lookup (:type item) scope))
               (str "type " (:name item) " already exists in scope"))
  (type-assert (apply distinct? (map :name (:members item)))
               "not all struct member names are distinct")
  (let [item-name (:name item)
        checked-members (map #(check-item % scope) (:members item))
        ; Add the struct type into scope
        scope-with-struct (type-definition/add-to-scope item scope)
        ; Add a member function for each member
        checked-scope (loop [members checked-members
                             new-scope scope-with-struct]
                        (if (empty? members)
                          new-scope
                          (recur (rest members)
                                 (binding-declaration/add-to-scope
                                   (fabricate/function-declaration
                                     (str "." (-> members first :name :name))
                                     [(:name item-name)]
                                     (-> members first :type :value :name))
                                   new-scope))))]
    (assoc item
           :members checked-members
           :scope checked-scope)))

(defmethod check-item :struct-member
  [item scope]
  (assoc item :scope scope))

(defmethod check-item :new-expression
  [item scope]
  ; TODO: Tests
  (let [types (:values (:specialization-list item))
        new-type (first types)
        expected-type (type-definition/lookup new-type scope)
        values (:values item)
        expected-value-types (apply list (map :type (:members expected-type)))
        value-types (apply list (map #(expression/realize-type % scope) values))]
    (type-assert (= 1 (count types))
                 (str "invalid number of types in new expression" types))
    (type-assert (some? expected-type)
                 (str "unknown type for new expression " new-type))
    (type-assert (= (count values)
                    (count (:members expected-type)))
                 (str "invalid number of specifiers for new "
                      "(expected "
                      (count (:members expected-type))
                      " found "
                      (count values)
                      ")"))
    (type-assert (= (map type-declaration/strip value-types)
                    (map type-declaration/strip expected-value-types))
                 (str "invalid types specified to new expression "
                      "(expected "
                      expected-value-types
                      " found "
                      value-types
                      ")"))
    (assoc item :scope scope)))

(defmethod check-item :binding-definition
  [item scope]
  ; There is an optional type specifier which may be before the value
  (let [checked-name (check-item (:name item) scope)
        value (:value item)
        checked-val (check-item
                      value
                      ; Add a declaration before checking it. This allows
                      ; recursive functions to have a declaration of
                      ; themselves.
                      (if (= :lambda-definition (:kind value))
                        (binding-declaration/add-to-scope
                          (assoc item
                                 :type (expression/realize-type value scope))
                          scope)
                        scope))
        updated-item (assoc item
                            :name checked-name
                            :value checked-val)
        value-type (expression/realize-type checked-val (:scope checked-val))]
    (assoc updated-item
           :type value-type
           :scope (binding-definition/add-to-scope
                    updated-item
                    (:scope checked-val)))))

(defmethod check-item :macro-definition
  [item scope]
  (let [checked-name (check-item (:name item) scope)
        checked-item (assoc item :name checked-name)]
    (assoc checked-item
           :scope (macro-definition/add-to-scope checked-item scope))))

(defmethod check-item :macro-function-call
  [item scope]
  (check-item
    ; TODO: lookup macro based on name *and* signature
    (if-let [macro-definition (macro-definition/lookup (-> item :name :name)
                                                       scope)]
      (assoc item
             :kind :macro-call
             :definition macro-definition)
      (assoc item :kind :function-call))
    scope))

(defmethod check-item :macro-call
  [item scope]
  (let [definition (:definition item)
        new-scope (scope.util/new-empty scope) ; XXX: Scope from call site, not definition
        checked-args (check-item (assoc (:arguments definition)
                                        :actual-arguments (:arguments item))
                                 new-scope)
        checked-body (check {:cells (:body definition)}
                            (:scope checked-args))
        updated-def (assoc definition
                           :arguments checked-args
                           :body (:cells checked-body)
                           :scope (:scope checked-body))
        def-with-return (return/add-explicit-returns updated-def
                                                     (:scope checked-body))
        item-with-return (assoc item :definition def-with-return)]
    (-> ; To avoid cyclical deps, we pass in our type checking function.
        ; Macros may call back into the type checker, to process data into
        ; the AST. This cyclical relationship is intended and, to me, logical.
        (macro/evaluate (interpret.scope.prelude/create check)
                        [item-with-return]
                        (get-in item-with-return [:definition :scope]))
        :cells
        ; Pull macro call out of cell wrapper
        first)))

(defmethod check-item :syntax-definition
  [item scope]
  ; TODO
  (assoc item :scope scope))

(defmethod check-item :escaped-item
  [item scope]
  (type-assert (scope.util/lookup (fn [k s] (true? (s k))) :in-macro? scope)
               "Cannot evaluate escape form outside of macro")
  (let [checked (check-item (:value item) scope)
        evaluated (escape/evaluate (interpret.scope.prelude/create check)
                                   [checked]
                                   (:scope checked))]
    (assoc evaluated :scope scope)))

(defmethod check-item :function-call
  [item scope]
  (loop [args (:arguments item)
         checked-args []
         new-scope scope]
    (if (empty? args)
      (let [checked-name (check-item (:name item) scope)
            args-with-returns (map #(return/add-parameter-returns % new-scope)
                                   checked-args)
            updated-item (assoc item
                                :name checked-name
                                :arguments args-with-returns)
            signature (expression/call-signature updated-item new-scope)]
        (assoc updated-item
               :scope new-scope
               :signature signature))
      (let [checked-arg (check-item (first args) new-scope)]
        (recur (rest args)
               (conj checked-args checked-arg)
               (:scope checked-arg))))))

; Bring the arguments into scope and type check.
(defmethod check-item :argument-list
  [item scope]
  ; Group arg names and types; pull out only the names and verify
  ; they're all distinct.
  (let [args (partition 2 (:values item))]
    (when (not-empty args)
      (type-assert (apply distinct? (map first args))
                   "not all parameter names are distinct"))
    (assoc item
           :scope
           (loop [remaining args
                  new-scope scope]
             (if (empty? remaining)
               new-scope
               (recur (rest remaining)
                      (binding-declaration/add-to-scope
                        (fabricate/binding-declaration
                          (ffirst remaining)
                          (-> remaining first second))
                        new-scope)))))))

(defmethod check-item :macro-argument-list
  [item scope]
  (let [args (:values item)
        actuals (:actual-arguments item)]
    (when (not-empty args)
      (type-assert (apply distinct? args)
                   "not all parameter names are distinct"))
    ; TODO: Check for explicit type and make sure the first is ast
    ; TODO: Type check each non-syntax argument
    ; TODO: Add param returns, if needed
    (let [checked-actuals (map #(check-item % scope) actuals)
          types (cons (fabricate/type "ast")
                      (map #(expression/realize-type % scope) checked-actuals))]
      (assoc item
             :actual-arguments checked-actuals
             :scope
             (reduce #(binding-declaration/add-to-scope
                        (apply fabricate/binding-declaration %2)
                        %1)
                     scope
                     (map vector args types))))))

(defmethod check-item :return-list
  [item scope]
  (let [returns (count (:values item))]
    (type-assert (<= returns 1) "multiple return types")
    (if (> returns 0)
      (let [expected-type (type-declaration/lookup
                            (first (:values item))
                            scope)]
        (type-assert expected-type "invalid return type")
        (assoc item
               :type expected-type
               :scope scope))
      (assoc item :scope scope))))

(defmethod check-item :if-expression
  [item scope]
  (let [checked-cond (check-item (-> item :condition :value) scope) ; TODO: Tests for function calls as condition
        item-with-checked-cond (assoc-in item
                                         [:condition :value] checked-cond)
        cond-type (expression/realize-type checked-cond scope)]
    (type-assert (= (type-declaration/strip cond-type)
                    {:kind :type
                     :value {:kind :identifier
                             :name "boolean"}})
                 (str "if expression condition must be boolean, not " cond-type))

    (let [checked-then (check {:cells [(:value (:then item-with-checked-cond))]}
                              (scope.util/new-empty scope))
          updated-item (assoc-in item-with-checked-cond
                                 [:then :value]
                                 (-> checked-then :cells first))
          scoped-item (assoc updated-item :scope scope)]
      (if (contains? item-with-checked-cond :else)
        (let [checked-else (check {:cells [(:value (:else scoped-item))]}
                                  (scope.util/new-empty scope))
              updated-item (assoc-in scoped-item
                                     [:else :value]
                                     (-> checked-else :cells first))]
          updated-item)
        scoped-item))))

(defmethod check-item :passthrough [item scope]
  (assoc item :scope scope))

(defmethod check-item :default [item scope]
  (type-assert false (str "no type checking for '" item "'")))
