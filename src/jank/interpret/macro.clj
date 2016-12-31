(ns jank.interpret.macro
  (:require [jank.parse.fabricate :as fabricate]
            [jank.type.scope.type-declaration :as type-declaration]
            [jank.type.expression :as expression]
            [jank.type.scope.util :as scope.util]
            [jank.interpret.scope.prelude :as prelude]
            [jank.interpret.scope.value :as value])
  (:use jank.assert
        jank.debug.log))

; TODO: Move the non-macro stuff out into an evaluate ns

(defmulti evaluate-item
  "Interprets the specified item, interpreting any necessary arguments and
   dependencies. Interpreted values are associated as :interpreted-value"
  (fn [prelude item scope scope-values]
    (let [kind (:kind item)]
      (if (type-declaration/built-ins kind)
        :primitive
        kind))))

(defn evaluate
  ([prelude body scope] (evaluate prelude body scope (value/new-empty)))
  ([prelude body scope scope-values]
   (reduce (fn [acc elem]
             (let [item (evaluate-item prelude elem
                                       (:scope acc) (:scope-values acc))
                   ret (assoc acc
                              :cells (conj (:cells acc) item)
                              :scope (:scope item))]
               (update ret :scope-values (fn [old]
                                           (or (:scope-values item)
                                               old)))))
           {:cells []
            :scope scope
            :scope-values scope-values}
           body)))

(defmethod evaluate-item :macro-call
  [prelude item scope scope-values]
  ; TODO: If external, the function must be in prelude
  (let [argument-pairs (map #(vector (:name %1)
                                     (expression/realize-type %2 scope)
                                     (evaluate-item prelude %2
                                                    scope scope-values))
                            (get-in item [:definition :arguments :values])
                            ; Add a wrapped ast value which supports emplacing
                            ; checked code and looking up in the scope.
                            (cons {:kind :ast
                                   :scope scope
                                   :emplaced []}
                                  (get-in item [:definition :arguments :actual-arguments])))
        def-scope (get-in item [:definition :scope])
        updated-item (assoc item
                            :scope-values
                            (reduce (fn [acc [name type value]]
                                      (value/add-to-scope name type value
                                                          def-scope acc))
                                    scope-values
                                    argument-pairs))
        body (evaluate prelude
                       (get-in updated-item [:definition :body])
                       (get-in updated-item [:definition :scope]))
        ; If the macro returns a non-empty checked syntax, pull out its scope
        checked-scope (or (-> body
                              :cells last
                              :interpreted-value :interpreted-value
                              :emplaced last
                              :scope)
                          (:scope item))]
    (-> (assoc item :scope checked-scope)
        ; Update the definition's body, since it now contains the interpreted
        ; value and updated scope
        (assoc-in [:definition :body] (:cells body))
        (assoc-in [:definition :scope] (:scope body)))))

; TODO: Relocate to a new home
(defn prelude-signature [fn-name fn-signature]
  (let [argument-types (-> fn-signature :generics :values first :values)]
    {:name fn-name
     :argument-types argument-types}))

(defmethod evaluate-item :function-call
  [prelude item scope scope-values]
  (let [named? (= :identifier (:kind (:name item)))
        fn-name (when named?
                  (-> item :name :name))
        fn-signature (-> item :signature :value)
        prelude-signature (prelude-signature fn-name fn-signature)
        evaluated-arguments (map #(evaluate-item prelude %
                                                 scope scope-values)
                                 (:arguments item))
        ; TODO: Add an easy accessor for things like return type
        ret-type (-> fn-signature :generics :values last :values first)
        func (if-let [f (prelude prelude-signature)]
               f
               (let [item-type (expression/call-signature item scope)
                     ; The name can be a lambda definition itself, if invoked
                     ; directly.
                     matched (if named?
                               (value/lookup fn-name
                                             (type-declaration/strip item-type)
                                             scope scope-values)
                               (:name item))]
                 (internal-assert matched (str "unable to find value for " fn-name))
                 ; TODO: Refactor this into a proper function
                 (fn [& args]
                   ; The new scope needs to be based on the scope of lambda body
                   (let [body-scope (-> matched :body first :scope)
                         add-to-scope (fn [acc-values [item-name item-type item-value scope]]
                                        (value/add-to-scope item-name item-type item-value
                                                            body-scope acc-values))
                         argument-names (filter #(= (:kind %) :identifier)
                                                (-> matched :arguments :values))
                         name-type-values (map vector
                                               (map :name argument-names)
                                               (:argument-types prelude-signature)
                                               evaluated-arguments)
                         new-values (reduce add-to-scope scope-values name-type-values)
                         result (evaluate prelude (:body matched) body-scope new-values)]
                     (-> (:cells result)
                         last
                         :interpreted-value
                         :interpreted-value)))))]
    (interpret-assert func (str "unknown function " prelude-signature))
    ; TODO: Send scope-values into prelude functions
    (let [result (apply func scope (map :interpreted-value evaluated-arguments))]
      (assoc item
             :interpreted-value result
             :scope scope))))

(defmethod evaluate-item :if-expression
  [prelude item scope scope-values]
  (let [cond-expr (-> item :condition :value)
        cond-value (evaluate-item prelude cond-expr
                                  scope scope-values)
        then (-> item :then :value)
        else (-> item :else :value)]
    (assoc item
           :interpreted-value (-> (if (:interpreted-value cond-value)
                                    (evaluate-item prelude then
                                                   scope scope-values)
                                    (when else
                                      (evaluate-item prelude else
                                                     scope scope-values)))
                                  :interpreted-value)
           :scope scope)))

(defmethod evaluate-item :lambda-definition
  [prelude item scope scope-values]
  (assoc item
         :interpreted-value item
         :scope scope))

(defmethod evaluate-item :primitive
  [prelude item scope scope-values]
  (assoc item
         :interpreted-value (:value item)
         :scope scope))

(defmethod evaluate-item :identifier
  [prelude item scope scope-values]
  (let [overloads (value/lookup (:name item) scope scope-values)
        item-type (expression/realize-type item scope)
        matched (get overloads (type-declaration/strip item-type))]
    (internal-assert matched (str "unable to find value for " item))
    matched))

(defmethod evaluate-item :return
  [prelude item scope scope-values]
  (assoc item
         :interpreted-value (when-let [value (:value item)]
                              (evaluate-item prelude value
                                             scope scope-values))
         :scope scope))

(defmethod evaluate-item :syntax-definition
  [prelude item scope scope-values]
  ; TODO
  (assoc item
         :interpreted-value item
         :scope scope))

(defmethod evaluate-item :ast
  [prelude item scope scope-values]
  ; TODO
  (assoc item
         :interpreted-value item
         :scope scope))

(defmethod evaluate-item :binding-definition
  [prelude item scope scope-values]
  (assoc item
         :scope scope
         :scope-values (value/add-to-scope (-> item :name :name)
                                           (type-declaration/strip (:type item))
                                           (evaluate-item prelude (:value item)
                                                          scope scope-values)
                                           scope
                                           scope-values)))

(defmethod evaluate-item :type-declaration
  [prelude item scope scope-values]
  (assoc item :scope scope))

(defmethod evaluate-item :binding-declaration
  [prelude item scope scope-values]
  (assoc item :scope scope))

(defmethod evaluate-item :struct-definition
  [prelude item scope scope-values]
  ; TODO: Add member function definitions to scope
  (assoc item :scope scope))

(defmethod evaluate-item :default
  [prelude item scope scope-values]
  (interpret-assert false (str "no supported evaluation for '" item "'")))
