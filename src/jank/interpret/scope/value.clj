(ns jank.interpret.scope.value
  (:require [jank.type.scope.util :as util])
  (:use jank.assert
        jank.debug.log))

(defn add-to-scope
  [item-name value scope]
  (let [source-path (util/path #((:binding-declarations %2) %1) item-name scope)
        dest-path [:interpreted-values item-name]]
    (internal-assert (some? source-path)
                     (str "no path found for item " item-name))
    (internal-assert (nil? (get-in scope dest-path))
                     (str "interpreted value already exists for " item-name))
    (assoc-in scope dest-path value)))
