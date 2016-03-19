(ns jank.parse
  (:require [instaparse.core :as insta]
            [jank.parse.transform :as transform])
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
         error (pr-str (insta/get-failure parsed))
         _ (parse-assert (not (insta/failure? parsed))
                         "invalid syntax\n" error)
         transformed (insta/transform
                       {:integer (partial transform/read-single :integer)
                        :real (partial transform/read-single :real)
                        :boolean (partial transform/read-single :boolean)
                        :keyword (partial transform/single :keyword)
                        :type (partial transform/single :type)
                        :then (partial transform/single :then)
                        :else (partial transform/single :else)
                        :identifier transform/identifier
                        :specialization-list transform/specialization-list
                        :declare-statement transform/declare-statement
                        :binding-definition transform/binding-definition
                        :function-call transform/function-call
                        :lambda-definition transform/lambda-definition
                        :argument-list transform/argument-list
                        :return-list transform/return-list
                        :if-expression transform/if-expression}
                       parsed)]
     (pprint parsed)
     (pprint transformed)
     transformed)))
