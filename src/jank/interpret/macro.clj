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
  (fn [prelude item scope]
    (let [kind (:kind item)]
      (if (type-declaration/built-ins kind)
        :primitive
        kind))))

(defn evaluate
  [prelude body scope]
  (reduce #(let [item (evaluate-item prelude %2 (:scope %1))]
             (assoc %1
                    :cells (conj (:cells %1) item)
                    :scope (:scope item)))
          {:cells []
           :scope scope}
          body))

(defmethod evaluate-item :macro-call
  [prelude item scope]
  ; TODO: If external, the function must be in prelude
  (let [argument-pairs (map #(vector (:name %1)
                                     (evaluate-item prelude %2 scope))
                            (get-in item [:definition :arguments :values])
                            ; Add a wrapped ast value which supports emplacing
                            ; checked code and looking up in the scope.
                            (cons {:kind :ast
                                   :scope scope
                                   :emplaced []}
                                  (get-in item [:definition :arguments :actual-arguments])))
        updated-item (update-in item
                                [:definition :scope]
                                (fn [inner-scope]
                                  (reduce (fn [acc [name value]]
                                            (value/add-to-scope name value acc))
                                          inner-scope
                                          argument-pairs)))
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

(defmethod evaluate-item :function-call
  [prelude item scope]
  (let [fn-name (-> item :name :name)
        signature {:name fn-name
                   :argument-types (map (comp type-declaration/strip
                                              #(expression/realize-type % (:scope item)))
                                        (:arguments item))}
        arguments (map #(evaluate-item prelude % scope) (:arguments item))
        ; TODO: Add an easy accessor for things like return type
        ret-type (-> item
                     :signature :value
                     :generics :values last :values first)
        func (if-let [f (prelude signature)]
               f
               ; TODO: Same code as identifier lookup
               (let [overloads (value/lookup fn-name scope)
                     item-type (expression/call-signature item scope)
                     matched (get overloads (type-declaration/strip item-type))]
                 (internal-assert matched (str "unable to find value for " item))
                 ; TODO: Refactor this into a proper function
                 (fn [& args]
                   ; The new scope needs to be based on the scope of lambda body
                   ; TODO: This new-scope doesn't have any interpreted values from the current scope
                   (let [empty-scope (scope.util/new-empty (-> matched :body first :scope))
                         add-to-scope (fn [acc-scope [item-name item-type item-value]]
                                        (value/add-to-scope item-name
                                                            item-type
                                                            item-value
                                                            acc-scope))
                         argument-names (filter #(= (:kind %) :identifier)
                                                (-> matched :arguments :values))
                         name-type-values (map vector
                                               (map :name argument-names)
                                               (:argument-types signature)
                                               arguments)
                         new-scope (reduce add-to-scope empty-scope name-type-values)
                         ; TODO: The matched scope doesn't have argument names like it should
                         result (evaluate prelude (:body matched) new-scope)]
                     (-> (:cells result)
                         last
                         :interpreted-value
                         :interpreted-value)))))]
    (interpret-assert func (str "unknown function " signature))
    (let [result (apply func scope (map :interpreted-value arguments))]
      (assoc item
             :interpreted-value result
             :scope scope))))

(defmethod evaluate-item :if-expression
  [prelude item scope]
  (let [cond-expr (-> item :condition :value)
        cond-value (evaluate-item prelude cond-expr scope)
        then (-> item :then :value)
        else (-> item :else :value)]
    (assoc item
           :interpreted-value (-> (if (:interpreted-value cond-value)
                                    (evaluate-item prelude then scope)
                                    (when else
                                      (evaluate-item prelude else scope)))
                                  :interpreted-value)
           :scope scope)))

(defmethod evaluate-item :lambda-definition
  [prelude item scope]
  (assoc item
         :interpreted-value item
         :scope scope))

(defmethod evaluate-item :primitive
  [prelude item scope]
  (assoc item
         :interpreted-value (:value item)
         :scope scope))

(defmethod evaluate-item :identifier
  [prelude item scope]
  (let [overloads (value/lookup (:name item) scope)
        item-type (expression/realize-type item scope)
        matched (get overloads (type-declaration/strip item-type))]
    (internal-assert matched (str "unable to find value for " item))
    matched))

(defmethod evaluate-item :return
  [prelude item scope]
  (assoc item
         :interpreted-value (evaluate-item prelude (:value item) scope)
         :scope scope))

(defmethod evaluate-item :syntax-definition
  [prelude item scope]
  ; TODO
  (assoc item
         :interpreted-value item
         :scope scope))

(defmethod evaluate-item :ast
  [prelude item scope]
  ; TODO
  (assoc item
         :interpreted-value item
         :scope scope))

(defmethod evaluate-item :binding-definition
  [prelude item scope]
  (assoc item
         :scope (value/add-to-scope (-> item :name :name)
                                    (type-declaration/strip (:type item))
                                    (evaluate-item prelude (:value item) scope)
                                    scope)))

(defmethod evaluate-item :type-declaration
  [prelude item scope]
  (assoc item :scope scope))

(defmethod evaluate-item :binding-declaration
  [prelude item scope]
  (assoc item :scope scope))

(defmethod evaluate-item :struct-definition
  [prelude item scope]
  ; TODO: Add member function definitions to scope
  (assoc item :scope scope))

(defmethod evaluate-item :default
  [prelude item scope]
  (interpret-assert false (str "no supported evaluation for '" item "'")))
