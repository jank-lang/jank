(ns jank.interpret.scope.value
  (:require [jank.type.scope.util :as util])
  (:use jank.assert
        jank.debug.log))

(defn add-to-scope
  [item-name value scope]
  (let [path (util/path #((:binding-declarations %2) %1) item-name scope)]
    (internal-assert (some? path)
                     (str "No path found for item " item-name))
    ; TODO: Assert the value isn't already there
    (assoc-in scope
              [:interpreted-values item-name]
              value)))
