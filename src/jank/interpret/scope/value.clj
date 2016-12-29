(ns jank.interpret.scope.value
  (:require [jank.type.scope.util :as util])
  (:use jank.assert
        jank.debug.log))

(def lookup (partial util/lookup #(get (:interpreted-values %2) %1)))

; TODO: Rework how values are stored
; 1. give each new-scope an id, from incrementing an atom
; 2. store values in a map of name and id to map of types to values
; 3. pass value map and type scope as separate items

(defn add-to-scope
  [item-name item-type value scope]
  (let [dest-path [:interpreted-values item-name item-type]]
    (internal-assert (nil? (get-in scope dest-path))
                     (str "interpreted value already exists for " item-name))
    (assoc-in scope dest-path value)))
