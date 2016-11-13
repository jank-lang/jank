(ns jank.interpret.check-shim
  (:require [jank.parse :as parse]
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
      :string (str "\"" (:value item) "\"")
      :identifier (str (:name item))
      (str (:value item)))))

(defn unparse
  "Walks the syntax definition and 'unparses' it back into a string."
  [syntax-def]
  (apply str (walk/postwalk unparse-item syntax-def)))

(defn check
  "Takes a syntax definition, converts it to a string, reparses it as normal
   code, and type checks it. Returns the checked body in a syntax definition."
  [scope actual-check syntax-def]
  (let [unparsed (unparse syntax-def)
        parsed (parse/parse "" unparsed) ; Empty prelude
        checked (actual-check {:cells parsed} scope)]
    checked))
