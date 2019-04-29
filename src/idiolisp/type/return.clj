(ns idiolisp.type.return
  (:require [idiolisp.parse.fabricate :as fabricate]
            [idiolisp.type.expression :as expression]
            [idiolisp.type.scope.type-declaration :as type-declaration])
  (:use idiolisp.assert
        idiolisp.debug.log))

(defmulti add-explicit-returns
  "Adds explicit returns to if statements, lambdas, etc.
   Returns the modified item."
  (fn [item scope]
    (:kind item)))

(defmulti add-parameter-returns
  "Forces explicit returns for expressions being used as parameters.
   Not all items need to be modified when they're treated as parameters.
   The primary example of this is if expressions, which can be incomplete
   on their own, but must be complete when used as a parameter."
  (fn [item scope]
    (:kind item)))

(defn lambda-macro-helper [item expected-type scope]
  ; Don't bother redoing the work if we've already done it.
  ; The real reason we care is that this should only be done once per lambda,
  ; since the lambda only has access to its full scope once. If another item
  ; tries to do this again, with a lambda return, the appropriate scope will
  ; no longer be available.
  (if (= :return (-> item :body last :kind))
    [item expected-type]
    ; No return type means no implicit returns are generated. Nice.
    (if (nil? expected-type)
      [item expected-type]
      (let [updated-body (add-explicit-returns {:kind :body
                                                :values (:body item)}
                                               scope)
            body-type (expression/realize-type (last (:values updated-body))
                                               scope)
            ; Allow deduction
            deduced-type (if (type-declaration/auto? expected-type)
                           body-type
                           expected-type)
            updated-item (assoc item :body (:values updated-body))]
        (type-assert (= (type-declaration/strip deduced-type)
                        (type-declaration/strip body-type))
                     (str "expected return type of "
                          deduced-type
                          ", found "
                          body-type))
        [updated-item deduced-type]))))

(defmethod add-explicit-returns :lambda-definition
  [item scope]
  (let [[updated-item
         deduced-type] (lambda-macro-helper item
                                            (-> item :return :values first)
                                            scope)]
    (assoc-in updated-item
              [:return :values] [deduced-type])))

(defmethod add-explicit-returns :macro-definition
  [item scope]
  (-> (lambda-macro-helper item (fabricate/type "ast") scope)
      first))

(defmethod add-explicit-returns :if-expression
  [item scope]
  (type-assert (contains? item :else) "no else statement")

  (let [then-body (:values (add-explicit-returns
                             {:kind :body
                              :values [(:value (:then item))]}
                             scope))
        else-body (:values (add-explicit-returns
                             {:kind :body
                              :values [(:value (:else item))]}
                             scope))]
    (internal-assert (not-empty then-body)
                     "no return value in if/then expression")
    (internal-assert (not-empty else-body)
                     "no return value in if/else expression")
    (internal-assert (= 1 (count then-body))
                     "malformed if/then body")
    (internal-assert (= 1 (count else-body))
                     "malformed if/else body")

    (let [then-type (expression/realize-type (last then-body) scope)
          else-type (expression/realize-type (last else-body) scope)]
      (type-assert (= (type-declaration/strip then-type)
                      (type-declaration/strip else-type))
                   (str "incompatible if then/else types "
                        then-type
                        " and "
                        else-type))

      (assoc (assoc-in (assoc-in item [:then :value] (first then-body))
                       [:else :value] (first else-body))
             :type then-type))))

(defmethod add-explicit-returns :body
  [item scope]
  (let [body (:values item)]
    (let [updated-last (add-explicit-returns (last body) scope)]
      (assoc item :values (concat (butlast body)
                                  [{:kind :return
                                    :value updated-last
                                    :scope scope}])))))

(defmethod add-explicit-returns :default
  [item scope]
  item)

(defmethod add-parameter-returns :if-expression
  [item scope]
  (add-explicit-returns item scope))

(defmethod add-parameter-returns :default
  [item scope]
  item)
