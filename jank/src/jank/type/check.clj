(ns jank.type.check)

(defn lookup-declaration [decl-name scope]
  "Recursively looks through the hierarchy of scopes for the declaration."
  (loop [current-scope scope]
    (when current-scope
      ; TODO: Should be a vector of overloads
      (if-let [found (find (:declarations current-scope) decl-name)]
        found
        (recur (:parent current-scope))))))

(defn validate-declaration [decl-name decl-type scope]
  "Looks up a declaration, if any, and verifies that the provided
   declaration has a matching type. Returns the decl or nil, if none is found."
  (let [decl (lookup-declaration decl-name scope)]
    (when (some? decl)
      (assert (= (:type (second decl)) decl-type)
              (str "Declaration of "
                   decl-name
                   " doesn't match previous declarations: "
                   (:type decl)
                   " vs "
                   decl-type)))
    decl))

(defn add-declaration [item scope]
  "Finds, validates, and adds the provided declaration into the scope.
   Returns the updated scope."
  (let [decl-name (get-in item [1 1])
        decl-type (get-in item [2 1])
        decl (validate-declaration decl-name decl-type scope)]
    (cond
      (nil? decl)
      (update scope :declarations assoc decl-name {:type decl-type})
      :else
      scope)))

(defmulti check-item
  "Type checks the given expression. Returns a cons of the typed
   expression and the updated scope."
  (fn [item scope]
    (first item)))

(defmethod check-item :declare-statement [item scope]
  ; TODO: Add type information to the item
  (list item (add-declaration item scope)))

(defmethod check-item :function-definition [item scope]
  (list item scope))

(defmethod check-item :lambda-definition [item scope]
  (list item scope))

(defmethod check-item :binding-definition [item scope]
  (list item scope))

(defmethod check-item :function-call [item scope]
  (list item scope))

(defmethod check-item :argument-list [item scope]
  (list item scope))

(defmethod check-item :if-statement [item scope]
  (list item scope))

(defmethod check-item :list [item scope]
  (list item scope))

(defmethod check-item :string [item scope]
  (list item scope))

(defmethod check-item :integer [item scope]
  (list item scope))

(defmethod check-item :real [item scope]
  (list item scope))

(defmethod check-item :boolean [item scope]
  (list item scope))

(defmethod check-item :identifier [item scope]
  (list item scope))

(defmethod check-item :default [item scope]
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
    (println "scope:" scope)
    (cond
      (nil? item)
      (list (update parsed :cells (fn [_] checked)) scope)
      :else
      (do
        (let [[checked-item new-scope] (check-item item scope)]
          (recur (first remaining)
                 (rest remaining)
                 (conj checked checked-item)
                 new-scope))))))
