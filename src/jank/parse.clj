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

(defn transform-identifier [& args]
  (let [base {:kind :identifier
              :name (first args)}]
    (if (= 1 (count args))
      base
      (assoc base :generics (second args)))))

(defn transform-specialization-list [& args]
  {:kind :specialization-list
   :values args})

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

(defn transform-function-call [& args]
  {:kind :function-call
   :name (first args)
   :arguments (rest args)})

(defn transform-lambda-definition [& args]
  {:kind :lambda-definition
   :arguments (first args)
   :return (second args)
   :body (drop 2 args)})

(defn transform-argument-list [& args]
  {:kind :argument-list
   :values args})

(defn transform-return-list [& args]
  {:kind :return-list
   :values args})

(defn transform-if-expression [& args]
  (let [base {:kind :if-expression
              :condition (first args)
              :then (second args)}]
    (if (= 2 (count args))
      base
      (assoc base :else (nth args 2)))))

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
                       :keyword (partial transform-single :keyword)
                       :type (partial transform-single :type)
                       :identifier transform-identifier
                       :specialization-list transform-specialization-list
                       :declare-statement transform-declare
                       :binding-definition transform-bind
                       :function-call transform-function-call
                       :lambda-definition transform-lambda-definition
                       :argument-list transform-argument-list
                       :return-list transform-return-list
                       :if-expression transform-if-expression
                       ;:macro-definition pass
                       }
                      parsed)
       )
     parsed)))
