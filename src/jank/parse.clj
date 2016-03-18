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

(defn transform-identifier [& more]
  (let [base {:kind :identifier
              :name (first more)}]
    (if (= 1 (count more))
      base
      (assoc base :generics (second more)))))

(defn transform-specialization-list [& more]
  {:kind :specialization-list
   :values more})

(defn transform-declare [& more]
  (let [base {:kind :declare-statement
              :type (last more)}
        size (count more)]
    (if (= 2 size) ; Has identifier (declaring a binding)
      (assoc base :name (first more))
      base)))

(defn transform-bind [& more]
  (let [base {:kind :binding-definition
              :name (first more)
              :value (last more)}
        size (count more)]
    (if (= 3 size) ; Has type
      (assoc base :type (second more))
      base)))

(defn transform-function-call [& more]
  {:kind :function-call
   :name (first more)
   :arguments (rest more)})

(defn transform-lambda-definition [& more]
  {:kind :lambda-definition
   :arguments (first more)
   :return (second more)
   :body (drop 2 more)})

(defn transform-argument-list [& more]
  {:kind :argument-list
   :values more})

(defn transform-return-list [& more]
  {:kind :return-list
   :values more})

(defn transform-if-expression [& more]
  (let [base {:kind :if-expression
              :condition (first more)
              :then (second more)}]
    (if (= 2 (count more))
      base
      (assoc base :else (nth more 2)))))

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
