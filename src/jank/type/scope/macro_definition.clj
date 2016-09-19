(ns jank.type.scope.macro-definition
  (:require [jank.parse.fabricate :as fabricate]
            [jank.type.expression :as expression]
            [jank.type.scope.type-declaration :as type-declaration]
            [jank.type.scope.binding-declaration :as binding-declaration]
            [jank.type.scope.util :as util])
  (:use jank.assert
        jank.debug.log))

(def lookup (partial util/lookup #((:macro-definitions %2) %1)))

(defn add-to-scope
  [item scope]
  "Adds the macro to the scope and performs type checking on the
   initial value. Returns the updated scope."
  (let [item-name (:name (:name item))
        overloads (lookup item-name scope)
        item-type (expression/realize-type item scope)
        expected-type (fabricate/type-declaration "syntax")]
    (type-assert (nil? overloads)
                 (str "macro already exists " item-name))
    (update scope
            :macro-definitions assoc item-name item)))
