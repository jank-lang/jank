(ns jank.type.scope.type-definition
  (:require [jank.type.scope.type-declaration :as type-declaration]
            [jank.type.scope.util :as util])
  (:use clojure.walk
        clojure.pprint
        jank.assert))

(def lookup (partial util/lookup #((:type-definitions %2) %1)))

(defn add-to-scope
  [item scope]
  (internal-assert (some? (:type item))
                   (str "item has no type " item))
  (let [with-decl (type-declaration/add-to-scope item scope)]
    (update with-decl :type-definitions conj (:type item))))
