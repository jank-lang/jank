(ns jank.type.check)

; TODO
; Add :type
; Add :scope {:parent {}
;             :bindings []}
(defn check [parsed]
  "Builds type information on the parsed source. Returns
   the typed source and the top-level scope."
  (loop [item (first (:cells parsed))
         remaining (rest (:cells parsed))
         checked []
         scope {}]
    (cond
      (nil? item)
      (list (update parsed :cells (fn [_] checked)) scope)
      :else
      (recur (first remaining)
             (rest remaining)
             (conj checked item)
             scope))))
