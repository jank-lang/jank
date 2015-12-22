(ns jank.type.check
  (:require [jank.type.declaration :as declaration]
            [jank.type.binding :as binding]
            [jank.type.expression :as expression])
  (:use clojure.pprint
        jank.assert))

(defmulti check-item
  "Type checks the given expression. Returns a cons of the typed
   expression and the updated scope."
  (fn [item scope]
    (first item)))

(defn empty-scope
  "Builds an empty type scope."
  ([]
   (empty-scope nil))
  ([parent]
   {:parent parent
    :binding-declarations {}
    :binding-definitions {}
    :type-declarations #{}}))

(defn check
  "Builds type information on the parsed source. Returns
   a list of the typed source and the top-level scope."
  ([parsed]
   (check parsed (empty-scope)))
  ([parsed parent-scope]
   ;(pprint (list "parsed:" parsed))
   (loop [item (first (:cells parsed))
          remaining (rest (:cells parsed))
          checked []
          scope parent-scope]
     ;(pprint (list "scope:" scope))
     (if (nil? item)
       (list (update parsed :cells (fn [_] checked)) scope)
       (let [[checked-item new-scope] (check-item item scope)]
         (recur (first remaining)
                (rest remaining)
                (conj checked checked-item)
                new-scope))))))

(defmethod check-item :declare-statement [item scope]
  (list item (declaration/add-to-scope item scope)))

(defmethod check-item :lambda-definition [item scope]
  (let [args (second item)
        returns (nth item 2)]
    ; TODO: Update item with checked return
    (check {:cells (drop 3 item)}
           (empty-scope
             (second (check-item returns
                                 (second (check-item args scope))))))
    (list item scope)))

(defmethod check-item :binding-definition [item scope]
  ; There is an optional type specifier which may be before the value
  (let [has-type (= 4 (count item))
        value-index (if has-type 3 2)
        [checked-val checked-scope] (check-item (nth item value-index) scope)
        updated-item (update-in item [value-index] (fn [_] checked-val))
        item-without-type (if has-type
                            (remove #(= % (nth updated-item 2)) updated-item)
                            updated-item)]
    ; Remove the optional type before it gets sent to codegen
    (list (apply vector item-without-type)
          (binding/add-to-scope updated-item checked-scope))))

(defmethod check-item :function-call [item scope]
  "Check the type of each argument and try to realize the resulting
   function type."
  (loop [args (drop 2 item)
         checked-args []
         new-scope scope]
    (if (empty? args)
      (let [checked-item (into [(first item) (second item)] checked-args)]
        (internal-assert (some? (expression/realize-type checked-item scope))
                         "invalid function type")
        (list checked-item new-scope))
      (let [[checked-arg checked-scope] (check-item (first args) new-scope)]
        (recur (rest args)
               (conj checked-args checked-arg)
               checked-scope)))))

(defmethod check-item :argument-list [item scope]
  "Bring the arguments into scope and type check."
  (let [args (partition 2 (rest item))]
    (when (not-empty args)
      (type-assert (distinct (map first args))
                   "not all parameter names are distinct"))
    (list item
          (loop [remaining args
                 new-scope scope]
            (if (empty? remaining)
              new-scope
              (recur (rest remaining)
                     (declaration/add-to-scope
                       (vec (cons :binding-declaration (first remaining)))
                       new-scope)))))))

(defmethod check-item :return-list [item scope]
  (let [returns (count (rest item))]
    (not-yet-implemented type-assert (<= (count (rest item)) 1)
                         "multiple return types")
    (when (> returns 0)
      (type-assert (declaration/lookup-type
                     (first (declaration/shorten-types (rest item))) scope)
                   "invalid return type"))
    (list item scope)))

(defmethod check-item :if-expression [item scope]
  (let [cond-type (expression/realize-type (get-in item [1 1]) scope)]
    (type-assert (= cond-type '("boolean"))
                 (str "if expression condition must be boolean, not " cond-type))
    (let [[checked-then then-scope] (check {:cells (rest (get-in item [2]))}
                                           (empty-scope scope))
          updated-item (update-in item [2]
                                  (fn [_] (into [:then] (:cells checked-then))))]
      (if (> (count item) 3) ; There's an else
        (let [[checked-else else-scope] (check {:cells (rest (get-in item [3]))}
                                               (empty-scope scope))]
          (list (update-in updated-item [3]
                           (fn [_] (into [:else] (:cells checked-else)))) scope))
        (list updated-item scope)))))

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
  (type-assert false (str "no type checking for '" item "'")))
