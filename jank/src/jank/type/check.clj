(ns jank.type.check
  (:require [jank.type.declaration :as declaration :refer [add-to-scope]]))

(defmulti check-item
  "Type checks the given expression. Returns a cons of the typed
   expression and the updated scope."
  (fn [item scope]
    (first item)))

(defmethod check-item :declare-statement [item scope]
  ; TODO: Add type information to the item
  (list item (declaration/add-to-scope item scope)))

(defmethod check-item :function-definition [item scope]
  (list item scope))

(defmethod check-item :lambda-definition [item scope]
  (list item scope))

(defmethod check-item :binding-definition [item scope]
  (list item scope))

(defmethod check-item :function-call [item scope]
  (println "--" item)
  (list item scope))

(defmethod check-item :argument-list [item scope]
  (list item scope))

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

(defn empty-scope []
  "Builds an empty type scope."
  {:binding-declarations {}
   :type-declarations #{}})

(defn check [parsed]
  "Builds type information on the parsed source. Returns
   a cons of the typed source and the top-level scope."
  (loop [item (first (:cells parsed))
         remaining (rest (:cells parsed))
         checked []
         scope (empty-scope)]
    (println "scope:" scope)
    (if (nil? item)
      (list (update parsed :cells (fn [_] checked)) scope)
      (let [[checked-item new-scope] (check-item item scope)]
        (recur (first remaining)
               (rest remaining)
               (conj checked checked-item)
               new-scope)))))
