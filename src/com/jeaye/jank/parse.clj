(ns com.jeaye.jank.parse
  (:require [instaparse.core :as insta]
            [com.jeaye.jank
             [log :refer [pprint]]
             [assert :refer [parse-assert]]]
            [com.jeaye.jank.parse
             [transform :as transform]]))

(insta/defparser whitespace-or-comments-parser
  (clojure.java.io/resource "neo-whitespace-grammar"))

(insta/defparser parser
  (clojure.java.io/resource "neo-grammar")
  :auto-whitespace whitespace-or-comments-parser
  :output-format :enlive)

(defn add-meta [input parsed]
  (insta/add-line-and-column-info-to-metadata input parsed))

(defn parse
  "Runs the provided resource file through instaparse and transforms from hiccup
   to hickory. Returns the generated syntax tree."
  [prelude file input]
  ;(pprint "parsing" input)
   (let [parsed (pprint "parsed raw" (parser input))
         error (pr-str (insta/get-failure parsed))
         _ (parse-assert (not (insta/failure? parsed))
                         "invalid syntax\n" error)
         parsed-with-meta (add-meta input parsed)
         _ (pprint "parsed" parsed-with-meta)
         transformed (transform/walk file parsed-with-meta)]
     (pprint "transformed" transformed)
     {::file file
      ::tree (into prelude transformed)}))

(defn parses [source & args]
  (apply insta/parses parser source args))

(def prelude (->> (clojure.java.io/resource "neo-prelude.jank")
                  slurp
                  (parse [] "<prelude>")
                  ::tree))
