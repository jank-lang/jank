(ns jank.type.scope.binding-definition
  (:require [jank.type.expression :as expression]
            [jank.type.scope.type :as type]
            [jank.type.scope.binding-declaration :as binding-declaration]
            [jank.type.scope.util :as util])
  (:use clojure.pprint
        jank.assert))

(def lookup (partial util/lookup :binding-definitions))

(defn add-to-scope
  [item scope]
  "Adds the binding to the scope and performs type checking on the
   initial value. Returns the updated scope."
  (let [item-name (:name (:name item))
        overloads (lookup item-name scope)
        has-type (contains? item :type)
        item-type (expression/realize-type (:value item) scope)
        expected-type (if has-type
                        (type/lookup (:type item) scope)
                        item-type)
        function? (type/function? item-type)
        overload-matches (if function?
                           (binding-declaration/match-overload item-type (second overloads))
                           nil)]
    (type-assert (or (nil? overloads)
                     (and type/function? (empty? overload-matches)))
                 (str "binding already exists " item-name))
    (type-assert (= (type/strip expected-type)
                    (type/strip item-type))
                 (str "expected binding type "
                      expected-type
                      ", found type "
                      item-type))

    (let [scope-with-decl (binding-declaration/add-to-scope
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
