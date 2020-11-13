(ns jank.interpret.scope.value
  (:require [jank.type.scope.util :as util])
  (:use jank.assert
        jank.debug.log))

(defn new-empty []
  {})

(defn- name-id [item-name scope]
  {:name item-name
   :id (:id scope)})

(defn lookup
  ; TODO: Document
  [item-name item-type scope scope-values]
  (util/lookup (fn [_ cur-scope]
                 (get-in scope-values [(name-id item-name cur-scope)
                                       item-type]))
               :ignored
               scope))

(defn add-to-scope
  [item-name item-type value scope scope-values]
  (let [dest-path [(name-id item-name scope) item-type]]
    (internal-assert (nil? (get-in scope-values dest-path))
                     (str "interpreted value already exists for " item-name))
    (assoc-in scope-values dest-path value)))
