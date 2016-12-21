(ns jank.interpret.check-shim
  (:require [jank.parse :as parse]
            [jank.type.scope.util :as scope.util]
            [clojure.walk :as walk]
            [clojure.string :as string])
  (:use jank.assert
        jank.debug.log))

(defn new-macro-scope
  "Returns a scope that's marked as being in a macro"
  [parent]
  (assoc (scope.util/new-empty parent)
         :in-macro? true))

(declare unparse)

(defn unparse-item
  "'Unparses' a single item, if it's a map, into a string. Any syntax escape
   items are evaluated and the result is emplaced and unparsed."
  [item actual-check scope]
  (if (not (map? item))
    item
    (condp = (:kind item)
      :syntax-definition (apply str (:body item))
      :syntax-list (str "(" (string/join " " (:body item)) ")")
      :syntax-item (:value item)
      :syntax-escaped-item (let [s (str "(escape " (:value item) ")")
                                 parsed (parse/parse "" s)
                                 checked (actual-check {:cells parsed}
                                                       (new-macro-scope scope))
                                 last-cell (-> checked :cells last)]
                             (unparse last-cell actual-check scope))
      :string (str "\"" (:value item) "\"")
      :identifier (str (:name item))
      (str (:value item)))))

(defn unparse
  "Walks the syntax definition and 'unparses' it back into a string."
  [syntax-def actual-check scope]
  (apply str (walk/postwalk #(unparse-item % actual-check scope)
                            (clean-scope syntax-def))))

(defn check
  "Takes a syntax definition, converts it to a string, reparses it as normal
   code, and type checks it. Returns the checked body in a syntax definition."
  [scope actual-check syntax-def]
  (let [unparsed (unparse syntax-def actual-check scope)
        ; Empty prelude, since the scope already includes it
        parsed (parse/parse "" unparsed)
        checked (actual-check {:cells parsed} ; TODO: Prevent usage of unescaped items in macro scope
                              (new-macro-scope scope))]
    checked))
