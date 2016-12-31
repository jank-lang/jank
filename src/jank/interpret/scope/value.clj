(ns jank.interpret.scope.value
  (:require [jank.type.scope.util :as util])
  (:use jank.assert
        jank.debug.log))

; TODO: Rework how values are stored
; 1. give each new-scope an id, from incrementing an atom
; 2. store values in a map of name and id to map of types to values
; 3. pass value map and type scope as separate items

(defn new-empty []
  {})

(defn- name-id [item-name scope]
  {:name item-name
   :id (:id scope)})

(defn lookup
  ; TODO: Document
  ([item-name item-type scope scope-values]
   (get (lookup item-name scope scope-values) item-type))
  ([item-name scope scope-values]
   (util/lookup (fn [_ cur-scope]
                  (get scope-values (name-id item-name cur-scope)))
                :ignored
                scope)))

(defn add-to-scope
  [item-name item-type value scope scope-values]
  (let [dest-path [(name-id item-name scope) item-type]]
    (internal-assert (nil? (get-in scope-values dest-path))
                     (str "interpreted value already exists for " item-name))
    (assoc-in scope-values dest-path value)))
