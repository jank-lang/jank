(ns jank.interpret.scope.value
  (:require [jank.type.scope.util :as util])
  (:use jank.assert
        jank.debug.log))

(def lookup (partial util/lookup #(get (:interpreted-values %2) %1)))

(defn add-to-scope
  [item-name item-type value scope]
  (let [dest-path [:interpreted-values item-name item-type]]
    (internal-assert (nil? (get-in scope dest-path))
                     (str "interpreted value already exists for " item-name))
    (assoc-in scope dest-path value)))
