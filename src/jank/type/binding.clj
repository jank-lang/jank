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
    #(when (= (-> %2 :value :generics :values first)
              (-> item-type :value :generics :values first))
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
  (let [item-name (:name (:name item))
        overloads (lookup item-name scope)
        has-type (contains? item :type)
        item-type (expression/realize-type (:value item) scope)
        expected-type (if has-type
                        (declaration/lookup-type (:type item) scope)
                        item-type)
        function? (declaration/function? item-type)
        overload-matches (if function?
                           (match-overload item-type (second overloads))
                           nil)]
    (type-assert (or (nil? overloads)
                     (and function? (empty? overload-matches)))
                 (str "binding already exists " item-name))
    (type-assert (= (declaration/strip-type expected-type)
                    (declaration/strip-type item-type))
                 (str "expected binding type "
                      expected-type
                      ", found type "
                      item-type))

    (let [scope-with-decl (declaration/add-to-scope
                            {:kind :binding-declaration
                             :name {:kind :identifier
                                    :name item-name}
                             :type item-type
                             :external? false}
                            scope)]
      (if (nil? overloads)
        (update
          scope-with-decl
          :binding-definitions assoc item-name [item-type])
        (update-in
          scope-with-decl
          [:binding-definitions item-name] conj item-type)))))
