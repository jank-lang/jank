(ns jank.interpret.macro
  (:use jank.assert
        jank.debug.log))

(def prelude {:functions {{:name "print!"
                           :argument-types [:string]} #(println %)}})

(defmulti evaluate-item
  (fn [item env]
    (:kind item)))

(defn evaluate
  ([body] (evaluate body prelude))
  ([body env]
   (reduce #(let [item (evaluate-item %2 (:env %1))]
              (assoc %1
                     :cells (conj (:cells %1) item)
                     :env (:env item)))
           {:cells []
            :env env}
           body)))

(defmethod evaluate-item :macro-call
  [item env]
  (pprint "evaluating macro " (clean-scope item) env)
  ; TODO: if external, the function must be in prelude
  ; TODO: Add arguments to env
  ; TODO: (assoc item [:interpreted :value] ...)
  (let [body (evaluate (get-in item [:definition :body]) env)]
    (-> (assoc-in item [:definition :body] (:cells body))
        (assoc :env (:env body)))))

(defmethod evaluate-item :function-call
  [item env]
  (pprint "evaluating function " (clean-scope item) env)
  (let [signature {:name (-> item :name :name)
                   :argument-types (map :kind (:arguments item))}
        arguments (map #(evaluate-item % env) (:arguments item))
        func (get (:functions env) signature)]
    (interpret-assert func (str "unknown function " signature))
    (apply func (map :value arguments))
    (assoc item :env env)))

(defmethod evaluate-item :string
  [item env]
  (assoc item :env env))

(defmethod evaluate-item :identifier
  [item env]
  ; TODO
  (assoc item :env env))

(defmethod evaluate-item :default
  [item env]
  (interpret-assert false (str "no supported evaluation for '" item "'")))
