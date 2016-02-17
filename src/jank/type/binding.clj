(ns jank.type.binding
  (:require [jank.type.expression :as expression]
            [jank.type.declaration :as declaration])
  (:use clojure.pprint
        jank.assert))

(defn match-overload
  "Looks through all overloads for one matching the provided type. Functions
   can't be overloaded by only return types. Returns a list of indices into
   the overloads sequence representing each match."
  [item-type overloads]
  (keep-indexed
    #(when (= (second (second (:type %2)))
              (second (second item-type)))
       %1)
    overloads))

(defn lookup
  "Recursively looks up a binding by name.
   Returns the binding, if found, or nil."
  [name scope]
  (loop [current-scope scope]
    (when current-scope
      (if-let [overloads (find (:binding-definitions current-scope) name)]
        overloads
        (recur (:parent current-scope))))))

(defn add-to-scope [item scope]
  "Adds the binding to the scope and performs type checking on the
   initial value. Returns the updated scope."
  (let [item-name (second (second item))
        overloads (lookup item-name scope)
        has-type (= 4 (count item))
        item-type (expression/realize-type (last item) scope)
        expected-type (if has-type
                        (declaration/lookup-type
                          (declaration/shorten-types (nth item 2))
                          scope)
                        item-type)
        function (declaration/function? item-type)
        overload-matches (if function
                           (match-overload item-type (second overloads))
                           nil)]
    (type-assert (or (nil? overloads)
                     (and function (empty? overload-matches)))
                 (str "binding already exists " item-name))
    (type-assert (= expected-type item-type)
                 (str "expected binding type "
                      expected-type
                      ", found type "
                      item-type))

    (let [scope-with-decl (declaration/add-to-scope
                            [:declare-statement
                             [:identifier item-name]
                             item-type]
                            scope)]
      (if (nil? overloads)
        (update
          scope-with-decl
          :binding-definitions assoc item-name [{:type item-type}])
        (update-in
          scope-with-decl
          [:binding-definitions item-name] conj {:type item-type})))))
