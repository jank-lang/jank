(ns jank.interpret.scope.value
  (:require [jank.type.scope.util :as util])
  (:use jank.assert
        jank.debug.log))

(defn add-to-scope
  [item-name value scope]
  (let [path (util/path #((:binding-definitions %2) %1) item-name scope)]
    (internal-assert (not-empty path)
                     (str "No path found for item " item-name))
    (update-in scope
               (conj path :binding-definitions item-name)
               assoc :value value)))
