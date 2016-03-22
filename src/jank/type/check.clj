(ns jank.type.check
  (:require [jank.type.declaration :as declaration]
            [jank.type.binding :as binding]
            [jank.type.expression :as expression]
            [jank.type.return :as return])
  (:use clojure.pprint
        jank.assert))

; XXX: migrated
(defmulti check-item
  "Type checks the given expression. Returns a cons of the typed
   expression and the updated scope."
  (fn [item scope]
    (:kind item)))

; XXX: migrated
(defn empty-scope
  "Builds an empty type scope."
  ([]
   (empty-scope nil))
  ([parent]
   {:parent parent
    :binding-declarations {}
    :binding-definitions {}
    :type-declarations #{}}))

; XXX: migrated
(defn check
  "Builds type information on the parsed source. Returns
   a list of the typed source and the top-level scope."
  ([parsed]
   (check parsed (empty-scope)))
  ([parsed parent-scope]
   ;(pprint (list "parsed:" parsed))
   (loop [item (first (:cells parsed))
          remaining (rest (:cells parsed))
          checked-cells []
          scope parent-scope]
     ;(pprint (list "scope:" scope))
     (if (nil? item)
       (assoc parsed
              :cells checked-cells
              :scope scope)
       (let [checked-item (check-item item scope)]
         (recur (first remaining)
                (rest remaining)
                (conj checked-cells checked-item)
                (:scope checked-item)))))))

; XXX: migrated | tested
(defmethod check-item :declare-statement
  [item scope]
  (assoc item :scope (declaration/add-to-scope item scope)))

; XXX: migrated
(defmethod check-item :lambda-definition
  [item scope]
  (let [args (:arguments item)
        return (:return item)
        new-scope (empty-scope scope)
        checked-args (check-item args new-scope)
        checked-return (check-item return (:scope checked-args))
        checked-body (check {:cells (:body item)} (:scope checked-return))
        updated-item (assoc item :body checked-body)
        body-with-return (return/add-explicit-returns updated-item
                                                      (:scope checked-body))]
    (assoc item
           :arguments checked-args
           :return checked-return
           :body checked-body
           :scope new-scope)))

; XXX: migrated
(defmethod check-item :binding-definition
  [item scope]
  ; There is an optional type specifier which may be before the value
  (let [value (:value item)
        value-type (expression/realize-type value scope)
        checked-val (check-item
                      value
                      ; Add a declaration before checking it. This allows
                      ; recursive functions to have a declaration of
                      ; themselves.
                      (if (= :lambda-definition (:kind value))
                        (declaration/add-to-scope
                          (assoc item :type value-type)
                          scope)
                        scope))]
    (assoc item
           :value checked-val
           :type value-type
           :scope (binding/add-to-scope item (:scope checked-val)))))

; Check the type of each argument and try to realize the resulting
; function type.
; XXX: migrated
(defmethod check-item :function-call
  [item scope]
  (loop [args (:arguments item)
         checked-args []
         new-scope scope]
    (if (empty? args)
      (let [checked-name (check-item (:name item) scope)
            args-with-returns (map #(return/add-parameter-returns % new-scope)
                                   checked-args)
            signature (expression/call-signature item new-scope)]
        (assoc item
               :arguments args-with-returns
               :scope new-scope
               :signature signature))
      (let [checked-arg (check-item (first args) new-scope)]
        (recur (rest args)
               (conj checked-args checked-arg)
               (:scope checked-arg))))))

; Bring the arguments into scope and type check.
; XXX: migrated
(defmethod check-item :argument-list
  [item scope]
  (let [args (partition 2 (:values item))]
    (when (not-empty args)
      (type-assert (distinct (map first args))
                   "not all parameter names are distinct"))
    (assoc item
           :scope
           (loop [remaining args
                  new-scope scope]
             (if (empty? remaining)
               new-scope
               (recur (rest remaining)
                      (declaration/add-to-scope
                        {:kind :binding-declaration
                         :name (ffirst remaining)
                         :type (second (first remaining))}
                        new-scope)))))))

; XXX: migrated
(defmethod check-item :return-list
  [item scope]
  (let [returns (count (:values item))]
    (type-assert (<= returns 1) "multiple return types")
    (if (> returns 0)
      (let [expected-type (declaration/lookup-type
                            (first (declaration/shorten-types (:values item)))
                            scope)]
        (type-assert expected-type "invalid return type")
        (assoc item
               :type expected-type
               :scope scope))
      (assoc item :scope scope))))

; XXX: migrated
(defmethod check-item :if-expression
  [item scope]
  (let [cond-type (expression/realize-type (:condition item) scope)]
    (type-assert (= cond-type '("boolean"))
                 (str "if expression condition must be boolean, not " cond-type))
    (let [checked-then (check {:cells (:then item)} (empty-scope scope))
          updated-item (assoc item :then checked-then)]
      (if (contains? item :else)
        (let [checked-else (check {:cells (:else item)} (empty-scope scope))]
          (assoc item
                 :else checked-else
                 :scope scope))
        (assoc item :scope scope)))))

(defmethod check-item :list [item scope]
  item)

(defmethod check-item :string [item scope]
  item)

(defmethod check-item :integer [item scope]
  item)

(defmethod check-item :real [item scope]
  item)

(defmethod check-item :boolean [item scope]
  item)

(defmethod check-item :identifier [item scope]
  item)

(defmethod check-item :default [item scope]
  (type-assert false (str "no type checking for '" item "'")))
