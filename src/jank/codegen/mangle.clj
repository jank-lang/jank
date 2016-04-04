(ns jank.codegen.mangle
  (:require [jank.codegen.sanitize :as sanitize])
  (:use jank.assert
        jank.debug.log
        clojure.pprint))

; TODO: Hash mangled type
(defmulti mangle
  "Flattens the item into a string for use with name serialization."
  (fn [item]
    (:kind item)))

(defmethod mangle :type
  [item]
  (sanitize/sanitize-str (mangle (:value item))))

(defmethod mangle :identifier
  [item]
  (sanitize/sanitize-str
    (let [ret (str (:name item) "_t")]
      (if (contains? item :generics)
        (apply str ret (map mangle (-> item :generics :values)))
        ret))))

(defmethod mangle :specialization-list
  [item]
  (sanitize/sanitize-str
    (apply str (map mangle (:values item)))))

(defmethod mangle :binding-name
  [item]
  (sanitize/sanitize-str (str (:name (:name item))
                              (mangle (:type item)))))

(defmethod mangle :function-call
  [item]
  (sanitize/sanitize-str (str (:name (:name item))
                              (mangle (:signature item)))))

(defmethod mangle :default
  [item]
  (codegen-assert false (str "invalid item to mangle " item)))
