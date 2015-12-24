(ns jank.type.return
  (:require [jank.type.expression :as expression])
  (:use clojure.pprint
        jank.assert))

(defmulti add-explicit-returns
  "Adds explicit returns to if statements, lambdas, etc.
   Returns the modified item."
  (fn [item scope]
    (first item)))

;(defmethod add-explicit-returns :lambda-definition [item scope]
;  item)
;
(defmethod add-explicit-returns :if-expression [item scope]
  (type-assert (= 4 (count item)) "no else statement")

  (let [then-body (second (add-explicit-returns [:body (rest (nth item 2))]
                                                scope))
        else-body (second (add-explicit-returns [:body (rest (nth item 3))]
                                                scope))]
    (type-assert (not-empty then-body) "no return value in if/then expression")
    (type-assert (not-empty else-body) "no return value in if/else expression")

    (let [then-type (expression/realize-type
                      (add-explicit-returns (last then-body) scope)
                      scope)
          else-type (expression/realize-type
                      (add-explicit-returns (last else-body) scope)
                      scope)]
      (type-assert (= then-type else-type)
                   "incompatible if then/else types")
      (update-in
        (update-in item [2] (fn [_] [:then [:return (last then-body)]]))
        [3]
        (fn [_] [:else [:return (last else-body)]])))))

(defmethod add-explicit-returns :body [item scope]
  item)

(defmethod add-explicit-returns :default [item scope]
  item)
