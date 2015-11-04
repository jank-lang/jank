(ns jank.type.declaration)

(defn lookup [decl-name scope]
  "Recursively looks through the hierarchy of scopes for the declaration."
  (loop [current-scope scope]
    (when current-scope
      ; TODO: Should be a vector of overloads
      (if-let [found (find (:declarations current-scope) decl-name)]
        found
        (recur (:parent current-scope))))))

(defn validate [decl-name decl-type scope]
  "Looks up a declaration, if any, and verifies that the provided
   declaration has a matching type. Returns the decl or nil, if none is found."
  (let [decl (lookup decl-name scope)]
    (when (some? decl)
      ; TODO: Allow overloads
      (assert (= (:type (second decl)) decl-type)
              (str "Declaration of "
                   decl-name
                   " doesn't match previous declarations: "
                   (:type (second decl))
                   " vs "
                   decl-type)))
    decl))

(defn add-to-scope [item scope]
  "Finds, validates, and adds the provided declaration into the scope.
   Returns the updated scope."
  (let [decl-name (get-in item [1 1])
        decl-type (get-in item [2 1])
        decl (validate decl-name decl-type scope)]
    (cond
      (nil? decl)
      (update scope :declarations assoc decl-name {:type decl-type})
      :else
      scope)))
