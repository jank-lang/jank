(ns jank.type.return
  (:require [jank.type.expression :as expression])
  (:use clojure.pprint
        jank.assert))

(defmulti add-explicit-returns
  "Adds explicit returns to if statements, lambdas, etc.
   Returns the modified item."
  (fn [item scope]
    (first item)))

(defmethod add-explicit-returns :lambda-definition [item scope]
  (let [updated-body (add-explicit-returns [:body (drop 3 item)] scope)]
    (into [] (concat (take 3 item) (second updated-body)))))

(defmethod add-explicit-returns :if-expression [item scope]
  (type-assert (= 4 (count item)) "no else statement")

  (let [then-body (second (add-explicit-returns [:body (rest (nth item 2))]
                                                scope))
        else-body (second (add-explicit-returns [:body (rest (nth item 3))]
                                                scope))]

    (let [then-type (expression/realize-type (last then-body) scope)
          else-type (expression/realize-type (last else-body) scope)]
      (type-assert (= then-type else-type)
                   "incompatible if then/else types")
      (update-in
        (update-in item [2] (fn [_] [:then (last then-body)]))
        [3]
        (fn [_] [:else (last else-body)])))))

(defmethod add-explicit-returns :body [item scope]
  (let [body (second item)]
    (type-assert (not-empty body)
                 "expression body is empty and without return")

    (let [body-type (expression/realize-type
                      (add-explicit-returns (last body) scope)
                      scope)]
      (update-in item [1] (fn [_] (concat (butlast body)
                                          [[:return (last body)]]))))))

(defmethod add-explicit-returns :default [item scope]
  item)
