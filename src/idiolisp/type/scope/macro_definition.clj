(ns idiolisp.type.scope.macro-definition
  (:require [idiolisp.parse.fabricate :as fabricate]
            [idiolisp.type.expression :as expression]
            [idiolisp.type.scope.type-declaration :as type-declaration]
            [idiolisp.type.scope.binding-declaration :as binding-declaration]
            [idiolisp.type.scope.util :as util])
  (:use idiolisp.assert
        idiolisp.debug.log))

(def lookup (partial util/lookup #(get (:macro-definitions %2) %1)))

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
