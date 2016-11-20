(ns jank.interpret.check-shim
  (:require [jank.parse :as parse]
            [jank.type.scope.util :as scope.util]
            [clojure.walk :as walk]
            [clojure.string :as string])
  (:use jank.assert
        jank.debug.log))

(defn unparse-item
  "'Unparses' a single item, if it's a map, into a string."
  [item]
  (if (not (map? item))
    item
    (condp = (:kind item)
      :syntax-list (str "(" (string/join " " (:body item)) ")")
      :syntax-item (:value item)
      :syntax-escaped-item (str "(escape " (:value item) ")")
      :string (str "\"" (:value item) "\"")
      :identifier (str (:name item))
      (str (:value item)))))

(defn unparse
  "Walks the syntax definition and 'unparses' it back into a string."
  [syntax-def]
  (apply str (walk/postwalk unparse-item syntax-def)))

(defn new-macro-scope
  "Returns a scope that's marked as being in a macro"
  [parent]
  (assoc (scope.util/new-empty parent)
         :in-macro? true))

(defn check
  "Takes a syntax definition, converts it to a string, reparses it as normal
   code, and type checks it. Returns the checked body in a syntax definition."
  [scope actual-check syntax-def]
  (let [unparsed (unparse syntax-def)
        ; Empty prelude, since the scope already includes it
        parsed (parse/parse "" unparsed)
        checked (actual-check {:cells parsed}
                              (new-macro-scope scope))]
    checked))
