(ns jank.parse
  (:require [instaparse.core :as insta])
  (:use clojure.pprint
        jank.assert))

(def prelude (slurp (clojure.java.io/resource "prelude.jank")))

(def parser
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn parse
  "Runs the provided resource file through instaparse. Returns
   then generated syntax tree."
  ([resource] (parse prelude resource))
  ([pre resource]
   (let [parsed (parser (str pre resource))
         error (pr-str (insta/get-failure parsed))]
     (parse-assert (not (insta/failure? parsed))
                   "invalid syntax\n" error)
     parsed)))
