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
                        :string (partial transform/single :string)
                        :type (partial transform/single :type)
                        :condition (partial transform/single :condition)
                        :then (partial transform/single :then)
                        :else (partial transform/single :else)
                        :identifier transform/identifier
                        :specialization-list transform/specialization-list
                        :type-declaration (partial transform/declaration :type)
                        :binding-declaration (partial transform/declaration :binding)
                        :binding-definition transform/binding-definition
                        :struct-definition transform/struct-definition
                        :struct-member transform/struct-member
                        :function-call transform/function-call
                        :lambda-definition transform/lambda-definition
                        :argument-list transform/argument-list
                        :return-list transform/return-list
                        :if-expression transform/if-expression}
                       parsed)]
     ;(pprint parsed)
     ;(pprint transformed)
     transformed)))
