(ns jank.type.return
  (:require [jank.type.expression :as expression])
  (:use jank.assert))

(defmulti add-explicit-returns
  "Adds explicit returns to if statements, lambdas, etc.
   Returns the modified item."
  (fn [item scope]
    (first item)))

;(defmethod add-explicit-returns :lambda-definition [item scope]
;  item)
;
(defmethod add-explicit-returns :if-expression [item scope]
  (let [then-body (add-explicit-returns [:body (rest (nth item 2))] scope)
        else-body (add-explicit-returns [:body (rest (nth item 3))] scope)]
    (type-assert (not-empty then-body) "no return value in if/then expression")
    (type-assert (not-empty else-body) "no return value in if/else expression")

    (let [then-type (expression/realize-type (last then-body) scope)
          else-type (expression/realize-type (last else-body) scope)]
      (type-assert (= then-body else-body)
                   "incompatible if then/else types")
      item)))

(defmethod add-explicit-returns :body [item scope]
  item)

(defmethod add-explicit-returns :default [item scope]
  (println item)
  item)
