(ns jank.parse
  (:require [instaparse.core :as insta])
  (:use clojure.pprint
        jank.assert))

(def prelude (slurp (clojure.java.io/resource "prelude.jank")))

(def parser
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn transform-single [kind value]
  {:kind kind :value value})

(defn transform-declare [& args]
  (let [base {:kind :declare-statement
              :type (last args)}
        size (count args)]
    (if (= 2 size) ; Has identifier (declaring a binding)
      (assoc base :name (first args))
      base)))

(defn transform-bind [& args]
  (let [base {:kind :binding-definition
              :name (first args)
              :value (last args)}
        size (count args)]
    (if (= 3 size) ; Has type
      (assoc base :type (second args))
      base)))

(defn parse
  "Runs the provided resource file through instaparse. Returns
   then generated syntax tree."
  ([resource] (parse prelude resource))
  ([pre resource]
   (let [parsed (parser (str pre resource))
         error (pr-str (insta/get-failure parsed))]
     (parse-assert (not (insta/failure? parsed))
                   "invalid syntax\n" error)
     (pprint parsed)
     (pprint
       ; TODO: Convert numbers from strings
     (insta/transform {:integer (partial transform-single :integer)
                       :real (partial transform-single :real)
                       :boolean (partial transform-single :boolean)
                       :identifier (partial transform-single :identifier)
                       :keyword (partial transform-single :keyword)
                       :type (partial transform-single :type)
                       :declare-statement transform-declare
                       :binding-definition transform-bind
                       ;:lambda-definition pass
                       ;:macro-definition pass
                       ;:if-expression pass
                       ;:function-call pass
                       }
                      parsed)
       )
     parsed)))
