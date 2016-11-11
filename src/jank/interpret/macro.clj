(ns jank.interpret.macro
  (:require [jank.parse.fabricate :as fabricate]
            [jank.type.scope.type-declaration :as type-declaration]
            [jank.type.expression :as expression]
            [jank.interpret.scope.prelude :as prelude]
            [jank.interpret.scope.value :as value])
  (:use jank.assert
        jank.debug.log))

(defn wrap-value
  "Wrap a raw value (such as 4 or \"foo\") in a kinded map with the scope"
  [value scope]
  {:kind :wrapped-value
   :interpreted-value value
   :scope scope})

(defmulti evaluate-item
  "Interprets the specified item, interpreting any necessary arguments and
   dependencies. Interpreted values are associated as :interpreted-value"
  (fn [prelude item scope]
    ; TODO: Use a set
    (if (some (partial = (:kind item)) [:string :integer :boolean :real])
      :primitive
      (:kind item))))

(defn evaluate
  [prelude body scope]
  ;(pprint (clean-scope body))
  ; TODO: Return value of last form?
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
                            ; TODO: Add value for AST
                            (rest (get-in item [:definition :arguments :values]))
                            (get-in item [:definition :arguments :actual-arguments]))
        updated-item (update-in item
                                [:definition :scope]
                                (fn [inner-scope]
                                  (reduce (fn [acc [name value]]
                                            (value/add-to-scope name value acc))
                                          inner-scope
                                          argument-pairs)))
        body (evaluate prelude
                       (get-in updated-item [:definition :body])
                       (get-in updated-item [:definition :scope]))]
    (-> (assoc-in item [:definition :body] (:cells body))
        (assoc-in [:definition :scope] (:scope body)))))

(defmethod evaluate-item :function-call
  [prelude item scope]
  (let [signature {:name (-> item :name :name)
                   :argument-types (map (comp type-declaration/strip
                                              #(expression/realize-type % (:scope item)))
                                        (:arguments item))}
        arguments (map #(evaluate-item prelude % scope) (:arguments item))
        func (if-let [f (prelude signature)]
               f
               (not-yet-implemented interpret-assert "non-prelude functions"))]
    (interpret-assert func (str "unknown function " signature))
    (let [result (apply func (map :interpreted-value arguments))]
      (wrap-value result scope))))

(defmethod evaluate-item :primitive
  [prelude item scope]
  (assoc item
         :interpreted-value (:value item)
         :scope scope))

(defmethod evaluate-item :identifier
  [prelude item scope]
  ; TODO: If value hasn't been evaluated (may be a def), do so
  (value/lookup (:name item) scope))

(defmethod evaluate-item :return
  [prelude item scope]
  ; TODO
  (assoc item :scope scope))

(defmethod evaluate-item :syntax-definition
  [prelude item scope]
  ; TODO
  (assoc item
         :interpreted-value (:body item)
         :scope scope))

(defmethod evaluate-item :default
  [prelude item scope]
  (interpret-assert false (str "no supported evaluation for '" item "'")))
