(ns jank.type.check)

(defmulti check-item
  (fn [item]
    (first item)))

(defmethod check-item :function-definition [item]
  item)

(defmethod check-item :lambda-definition [item]
  item)

(defmethod check-item :binding-definition [item]
  item)

(defmethod check-item :function-call [item]
  item)

(defmethod check-item :argument-list [item]
  item)

(defmethod check-item :if-statement [item]
  item)

(defmethod check-item :list [item]
  item)

(defmethod check-item :string [item]
  item)

(defmethod check-item :integer [item]
  item)

(defmethod check-item :real [item]
  item)

(defmethod check-item :boolean [item]
  item)

(defmethod check-item :identifier [item]
  item)

(defmethod check-item :default [item]
  (assert false (str "no type checking for '" item "'")))

; TODO
; Add :type
; Add :scope {:parent {}
;             :bindings []}
(defn check [parsed]
  "Builds type information on the parsed source. Returns
   a cons of the typed source and the top-level scope."
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
             (conj checked (check-item item))
             scope))))
