(ns jank.type.return
  (:require [jank.type.expression :as expression])
  (:use clojure.pprint
        jank.assert))

(defmulti add-explicit-returns
  "Adds explicit returns to if statements, lambdas, etc.
   Returns the modified item."
  (fn [item scope]
    (first item)))

(defmulti add-parameter-returns
  "Forces explicit returns for expressions being used as parameters.
   Not all items need to be modified when they're treated as parameters.
   The primary example of this is if expressions, which can be incomplete
   on their own, but must be complete when used as a parameter."
  (fn [item scope]
    (first item)))

(defmethod add-explicit-returns :lambda-definition [item scope]
  (let [updated-body (add-explicit-returns [:body (drop 3 item)] scope)]
    ; TODO: Verify the type is correct
    (into [] (concat (take 3 item) (second updated-body)))))

(defmethod add-explicit-returns :if-expression [item scope]
  (type-assert (= 4 (count item)) "no else statement")

  (let [then-body (second (add-explicit-returns [:body (rest (nth item 2))]
                                                scope))
        else-body (second (add-explicit-returns [:body (rest (nth item 3))]
                                                scope))]
    (internal-assert (not-empty then-body)
                     "no return value in if/then expression")
    (internal-assert (not-empty else-body)
                     "no return value in if/else expression")

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
    (let [body-type (expression/realize-type
                      (add-explicit-returns (last body) scope)
                      scope)]
      (update-in item [1] (fn [_] (concat (butlast body)
                                          [[:return (last body)]]))))))

(defmethod add-explicit-returns :default [item scope]
  item)

(defmethod add-parameter-returns :if-expression [item scope]
  (add-explicit-returns item scope))

(defmethod add-parameter-returns :default [item scope]
  item)
