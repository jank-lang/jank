(ns jank.parse
  (:require [instaparse.core :as insta]))

(def parse
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn map-from [index func coll]
  "Maps from the specified index to the end of the collection.
   The unmapped items are retained."
  (vec (concat (take index coll)
               (map func (drop index coll)))))

(defmulti handle
  (fn [current ast]
    (first current)))

(defmethod handle :lambda-definition [current ast]
  ; (λ (args) (rets) ...)
  (map-from 3 #(handle %1 ast) current))

(defmethod handle :macro-definition [current ast]
  ; (macro name (types) (args) ...)
  (map-from 4 #(handle %1 ast) current))

(defmethod handle :binding-definition [current ast]
  ; (bind name value)
  (map-from 2 #(handle %1 ast) current))

(defmethod handle :function-call [current ast]
  ; (foo ...)
  (map-from 1 #(handle %1 ast) current))

(defmethod handle :default [current ast]
  current)
