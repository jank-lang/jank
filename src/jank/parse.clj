(ns jank.parse
  (:require [instaparse.core :as insta])
  (:use jank.assert))

(def parser
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn parse [resource]
  "Runs the provided resource file through instaparse. Returns
   then generated syntax tree."
  (let [parsed (parser resource)]
    (when (insta/failure? parsed)
      (parse-assert false))
    parsed))
