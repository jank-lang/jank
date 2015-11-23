(ns jank.type.binding
  (:require [jank.type.expression :as expression]
            [jank.type.declaration :as declaration])
  (:use clojure.pprint
        jank.assert))

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
  (let [item-name (second (second item))
        found (lookup item-name scope)
        has-type (= 4 (count item))
        item-type (declaration/shorten-types
                    (expression/realize-type (last item) scope))
        expected-type (if has-type
                        (declaration/lookup-type
                          (declaration/shorten-types (nth item 2))
                          scope)
                        item-type)
        function (declaration/function? item-type)]
    (type-assert (or (nil? found)
                     (and function
                          (= -1 (.indexOf (second found) {:type item-type}))))
                 (str "binding already exists " item-name))
    (type-assert (= expected-type item-type)
                 (str "expected binding type "
                      expected-type
                      ", found type "
                      item-type))

    (let [scope-with-decl (declaration/add-to-scope
                            [:declare-statement
                             [:identifier item-name]
                             [:type (into [:identifier] item-type)]]
                            scope)]
      (if (nil? found)
        (update
          scope-with-decl
          :binding-definitions assoc item-name [{:type item-type}])
        (update-in
          scope-with-decl
          [:binding-definitions item-name] conj {:type item-type})))))
