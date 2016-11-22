(ns jank.interpret.macro
  (:require [jank.parse.fabricate :as fabricate]
            [jank.type.scope.type-declaration :as type-declaration]
            [jank.type.expression :as expression]
            [jank.interpret.scope.prelude :as prelude]
            [jank.interpret.scope.value :as value])
  (:use jank.assert
        jank.debug.log))

; TODO: Move the non-macro stuff out into an evaluate ns

(defn wrap-value
  "Wrap a raw value (such as 4 or \"foo\") in a kinded map with the scope"
  [type value scope]
  {:kind :wrapped-value
   :type type
   :interpreted-value value
   :scope scope})

(defmulti evaluate-item
  "Interprets the specified item, interpreting any necessary arguments and
   dependencies. Interpreted values are associated as :interpreted-value"
  (fn [prelude item scope]
    (let [kind (:kind item)]
      (if (#{:string :integer :boolean :real} kind)
        :primitive
        kind))))

(defn evaluate
  [prelude body scope]
  ; TODO: Return value of last form?
  (reduce #(let [item (evaluate-item prelude %2 (:scope %1))]
             (assoc %1
                    :cells (conj (:cells %1) item)
                    :interpreted-value (:interpreted-value item)
                    :scope (:scope item)))
          {:cells []
           :interpreted-value nil
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
  (let [signature {:name (-> item :name :name)
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
               (not-yet-implemented interpret-assert "non-prelude functions"))]
    (interpret-assert func (str "unknown function " signature))
    (let [result (apply func scope (map :interpreted-value arguments))]
      (wrap-value ret-type result scope)))) ; TODO: Loses the expression information

(defmethod evaluate-item :primitive
  [prelude item scope]
  (pprint ":primitive"
  (assoc item
         :interpreted-value (:value item)
         :scope scope)))

(defmethod evaluate-item :identifier
  [prelude item scope]
  ; TODO: If value hasn't been evaluated (may be a def), do so
  (value/lookup (:name item) scope))

(defmethod evaluate-item :return
  [prelude item scope]
  (assoc item
         :interpreted-value (evaluate-item prelude (:value item) scope)
         :scope scope))

(defmethod evaluate-item :syntax-definition
  [prelude item scope]
  ; TODO
  (assoc item
         :interpreted-value (:body item)
         :scope scope))

(defmethod evaluate-item :ast
  [prelude item scope]
  ; TODO
  (assoc item
         :interpreted-value item
         :scope scope))

(defmethod evaluate-item :default
  [prelude item scope]
  (interpret-assert false (str "no supported evaluation for '" item "'")))
