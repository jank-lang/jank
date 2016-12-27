(ns jank.type.scope.util
  (:use jank.assert
        jank.debug.log))

(defn lookup
  "Recursively looks through the hierarchy of scopes for the item.
   Returns the first item found in the closest scope, not all."
  [finder item-name scope]
  (loop [current-scope scope]
    (when current-scope
      (if-let [found (finder item-name current-scope)]
        found
        (recur (:parent current-scope))))))

(defn lookup-all
  "Recursively looks through the hierarchy of scopes for the item.
   Returns all matching items in all scopes, from closest to furthest."
  [finder item-name scope]
  (loop [current-scope scope
         all []]
    (if current-scope
      (if-let [found (finder item-name current-scope)]
        (recur (:parent current-scope) (into all (second found)))
        (recur (:parent current-scope) all))
      all)))

(defn path
  [finder item-name scope]
  (loop [current-scope scope
         path []]
    (if current-scope
      (if (finder item-name current-scope)
        path
        (recur (:parent current-scope)
               (conj path :parent)))
      nil)))

(defn new-empty
  "Builds an empty type scope."
  ([]
   (new-empty nil))
  ([parent]
   {:parent parent
    :macro-definitions {}
    :binding-declarations {}
    :binding-definitions {}
    :type-declarations #{}
    :type-definitions #{}}))
