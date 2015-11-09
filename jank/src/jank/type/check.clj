(ns jank.type.check
  (:require [jank.type.declaration :as declaration :refer [add-to-scope]]
            [jank.type.expression :as expression :refer [realize-type]])
  (:use clojure.pprint))

(defmulti check-item
  "Type checks the given expression. Returns a cons of the typed
   expression and the updated scope."
  (fn [item scope]
    (first item)))

(defmethod check-item :declare-statement [item scope]
  (list item (declaration/add-to-scope item scope)))

(defmethod check-item :function-definition [item scope]
  (list item scope))

(defmethod check-item :lambda-definition [item scope]
  ; TODO: Recurse into check with the new scope
  (list item scope))

(defmethod check-item :binding-definition [item scope]
  ; Special case for function definitions
  (if (= (first (nth item 2)) :lambda-definition)
    (check-item (update-in item [0] (fn [x] :function-definition)))
    (list item scope)))

(defmethod check-item :function-call [item scope]
  (expression/realize-type item scope)
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
     (pprint (list "scope:" scope))
     (if (nil? item)
       (list (update parsed :cells (fn [_] checked)) scope)
       (let [[checked-item new-scope] (check-item item scope)]
         (recur (first remaining)
                (rest remaining)
                (conj checked checked-item)
                new-scope))))))
