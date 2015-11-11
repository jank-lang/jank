(ns jank.type.binding
  (:require [jank.type.expression :as expression])
  (:use clojure.pprint))

(defn lookup [name scope]
  "Recursively looks up a binding by name.
   Returns the binding, if found, or nil."
  (loop [current-scope scope]
    (when current-scope
      (if-let [found (find (:binding-definitions current-scope) name)]
        found
        (recur (:parent current-scope))))))

(defn add-to-scope [item scope]
  "Adds the binding to the scope and performs type checking on the
   initial value. Returns the updated scope."
  (let [name (second (second item))
        found (lookup name scope)
        type (expression/realize-type (nth item 2) scope)]
    (assert (nil? found) (str "binding already exists: " name))
    ; TODO: Allow overloads
    (update
      (update scope :binding-declarations assoc name {:type type})
      :binding-definitions assoc name {:type type})))
