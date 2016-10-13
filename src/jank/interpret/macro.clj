(ns jank.interpret.macro
  (:require [jank.parse.fabricate :as fabricate]
            [jank.type.scope.type-declaration :as type-declaration]
            [jank.type.expression :as expression]
            [jank.interpret.scope.value :as value])
  (:use jank.assert
        jank.debug.log))

; TODO: Merge this into the scope
(def prelude {{:name "print!"
                :argument-types [(fabricate/type "string")]} pprint
               {:name "print!"
                :argument-types [(fabricate/type "integer")]} pprint
               {:name "print!"
                :argument-types [(fabricate/type "real")]} pprint
               {:name "print!"
                :argument-types [(fabricate/type "boolean")]} pprint})

(defmulti evaluate-item
  (fn [item env]
    (:kind item)))

(defn evaluate
  ([item] (evaluate [item] prelude))
  ([body env]
   ;(pprint (clean-scope body))
   (reduce #(let [item (evaluate-item %2 (:env %1))]
              (assoc %1
                     :cells (conj (:cells %1) item)
                     :env (:env item)))
           {:cells []
            :env env}
           body)))

(defmethod evaluate-item :macro-call
  [item env]
  ; TODO: If external, the function must be in prelude
  ; TODO: Bring arguments into env
  (let [;arg-types (map #(expression/realize-type % (:scope item))
        ;               (:arguments))
        argument-values (map (comp #(evaluate-item % env) :value)
                             (:actual-arguments item))
        ;env-with-args (reduce #(assoc %1 )
        ;                      env
        ;                      (:arguments item))
        body (evaluate (get-in item [:definition :body]) env)]
    (-> (assoc-in item [:definition :body] (:cells body))
        (assoc :env (:env body)))))

(defmethod evaluate-item :function-call
  [item env]
  ;(pprint "evaluating function " (clean-scope item) env)
  (let [signature {:name (-> item :name :name)
                   :argument-types (map (comp type-declaration/strip
                                              #(expression/realize-type % (:scope item)))
                                        (:arguments item))}
        ; TODO: look up arguments
        arguments (map #(evaluate-item % env) (:arguments item))
        func (get env signature)]
    (interpret-assert func (str "unknown function " signature))
    (apply func (map :value arguments))
    (assoc item :env env)))

; TODO: Assoc values onto each of these items
(defmethod evaluate-item :string
  [item env]
  (assoc item :env env))

(defmethod evaluate-item :integer
  [item env]
  (assoc item :env env))

(defmethod evaluate-item :real
  [item env]
  (assoc item :env env))

(defmethod evaluate-item :boolean
  [item env]
  (assoc item :env env))

(defmethod evaluate-item :identifier
  [item env]
  ; TODO
  (assoc item :env env))

(defmethod evaluate-item :return
  [item env]
  ; TODO
  (assoc item :env env))

(defmethod evaluate-item :default
  [item env]
  (interpret-assert false (str "no supported evaluation for '" item "'")))
