(ns jank.type.check
  (:require [jank.type.declaration :as declaration]
            [jank.type.binding :as binding]
            [jank.type.expression :as expression])
  (:use clojure.pprint))

(defmulti check-item
  "Type checks the given expression. Returns a cons of the typed
   expression and the updated scope."
  (fn [item scope]
    (first item)))

(defn empty-scope
  "Builds an empty type scope."
  ([]
   (empty-scope nil))
  ([parent]
   {:parent parent
    :binding-declarations {}
    :binding-definitions {}
    :type-declarations #{}}))

(defn check
  "Builds type information on the parsed source. Returns
   a cons of the typed source and the top-level scope."
  ([parsed]
   (check parsed (empty-scope)))
  ([parsed parent-scope]
   (pprint (list "parsed:" parsed))
   (loop [item (first (:cells parsed))
          remaining (rest (:cells parsed))
          checked []
          scope parent-scope]
     (if (nil? item)
       (list (update parsed :cells (fn [_] checked)) scope)
       (let [[checked-item new-scope] (check-item item scope)]
         (recur (first remaining)
                (rest remaining)
                (conj checked checked-item)
                new-scope))))))

(defmethod check-item :declare-statement [item scope]
  (list item (declaration/add-to-scope item scope)))

(defmethod check-item :lambda-definition [item scope]
  (let [args (second item)
        returns (nth item 2)]
    (check {:cells (drop 3 item)}
           (empty-scope
             (second (check-item returns
                                 (second (check-item args scope))))))
    (list item scope)))

(defmethod check-item :binding-definition [item scope]
  (list item (binding/add-to-scope item scope)))

(defmethod check-item :function-call [item scope]
  "Check the type of each argument and try to realize the resulting
   function type."
  (loop [args (drop 2 item)
         checked-args []
         new-scope scope]
    (if (empty? args)
      (let [checked-item (into [(first item) (second item)] checked-args)]
        (expression/realize-type checked-item scope)
        (list checked-item new-scope))
      (let [[checked-arg checked-scope] (check-item (first args) new-scope)]
        (recur (rest args)
               (conj checked-args checked-arg)
               checked-scope)))))

(defmethod check-item :argument-list [item scope]
  "Bring the arguments into scope and type check."
  (list item
        (loop [args (partition 2 (rest item))
               new-scope scope]
          (if (empty? args)
            new-scope
            (recur (rest args)
                   (declaration/add-to-scope
                     ; TODO: Change to binding definition
                     ; Declare allows duplicates; we don't want that here.
                     (vec (cons :declare-statement (first args)))
                     new-scope))))))

(defmethod check-item :return-list [item scope]
  (let [returns (count (rest item))]
    (assert (<= (count (rest item)) 1)
            "unimplemented: multiple return types")
    (when (> returns 0)
      (assert (declaration/lookup-type
                (first (declaration/shorten-types (rest item))) scope)
              "invalid return type"))
    (list item scope)))

(defmethod check-item :if-statement [item scope]
  (list item scope))

(defmethod check-item :list [item scope]
  (list item scope))

(defmethod check-item :string [item scope]
  (list item scope))

(defmethod check-item :integer [item scope]
  (list item scope))

(defmethod check-item :real [item scope]
  (list item scope))

(defmethod check-item :boolean [item scope]
  (list item scope))

(defmethod check-item :identifier [item scope]
  (list item scope))

(defmethod check-item :default [item scope]
  (assert false (str "no type checking for '" item "'")))
