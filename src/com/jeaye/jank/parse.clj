(ns com.jeaye.jank.parse
  (:require [instaparse.core :as insta]
            [com.jeaye.jank
             [log :refer [pprint]]
             [assert :refer [parse-assert]]]
            [com.jeaye.jank.parse
             [transform :as transform]]))

;(def prelude (slurp (clojure.java.io/resource "prelude.jank")))
(def prelude "")

(insta/defparser whitespace-or-comments-parser
  (clojure.java.io/resource "neo-whitespace-grammar"))

(insta/defparser parser
  (clojure.java.io/resource "neo-grammar")
  :auto-whitespace whitespace-or-comments-parser)

(defn parse
  "Runs the provided resource file through instaparse and transforms from hiccup
   to hickory. Returns the generated syntax tree."
  ([resource] (parse prelude resource))
  ([pre resource]
   ;(pprint "parsing" (str pre resource))
   (let [parsed (parser (str pre resource))
         error (pr-str (insta/get-failure parsed))
         _ (parse-assert (not (insta/failure? parsed))
                         "invalid syntax\n" error)
         _ (pprint "parsed" parsed)
         transformed (insta/transform
                       {:integer (partial transform/read-single :integer)
                        :real (partial transform/read-single :real)
                        :boolean (partial transform/read-single :boolean)
                        :keyword (partial transform/keyword :unqualified)
                        :qualified-keyword (partial transform/keyword :qualified)
                        :string (partial transform/single :string)
                        :identifier (partial transform/single :identifier)
                        :binding-definition transform/binding-definition
                        :application transform/application
                        :fn-definition transform/fn-definition
                        :argument-list transform/argument-list}
                       parsed)]
     ;(pprint "transformed" transformed)
     transformed)))

(defn parses [source & args]
  (apply insta/parses parser source args))
