(ns com.jeaye.jank.parse
  (:require [instaparse.core :as insta]
            [clojure.java.io :as io]
            [com.jeaye.jank
             [log :refer [pprint]]
             [assert :refer [parse-assert]]]
            [com.jeaye.jank.parse
             [binding :as parse.binding]
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
  [prelude]
  ;(pprint "parsing" input)
   (let [input parse.binding/*input-source*
         parsed (pprint "parsed raw" (parser input))
         error (pr-str (insta/get-failure parsed))
         _ (parse-assert (not (insta/failure? parsed))
                         "invalid syntax\n" error)
         parsed-with-meta (add-meta input parsed)
         _ (pprint "parsed" parsed-with-meta)
         transformed (transform/walk parsed-with-meta)]
     ;(pprint "transformed" transformed)
     {::file parse.binding/*input-file*
      ::tree (into prelude transformed)}))

(defn parses [source & args]
  (apply insta/parses parser source args))

(def prelude-file "neo-prelude.jank")
(def prelude
  (->> (binding [parse.binding/*input-file* prelude-file
                 parse.binding/*input-source* (-> (io/resource prelude-file)
                                                  slurp)]
         (parse []))
       ::tree))
