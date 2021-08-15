(ns com.jeaye.jank.parse
  (:require [clojure.java.io :as io]
            [instaparse.core :as insta]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank
             [log :refer [pprint]]
             [assert :refer [incomplete-parse!]]]
            [com.jeaye.jank.parse
             [binding :as parse.binding]
             [transform :as parse.transform]]))

(insta/defparser parser
  (clojure.java.io/resource "neo-grammar")
  :output-format :enlive)

(defn add-meta [input parsed]
  (if (empty? parsed)
    parsed
    (insta/add-line-and-column-info-to-metadata input parsed)))

(defn parse
  "Runs the provided resource file through instaparse and transforms from hiccup
   to hickory. Returns the generated syntax tree."
  [prelude]
  ;(pprint "parsing" input)
  ; TODO: Assert that the proper bindings are present
   (let [input parse.binding/*input-source*
         parsed (parser input)
         ;_ (pprint "raw parsed" parsed)
         _ (when (insta/failure? parsed)
             (incomplete-parse! (insta/get-failure parsed)))
         parsed-with-meta (add-meta input parsed)
         ;_ (pprint "parsed" parsed-with-meta)
         transformed (parse.transform/walk parsed-with-meta)]
     ;(pprint "transformed" transformed)
     (into prelude transformed)))

(defn parses [source & args]
  (apply insta/parses parser source args))

(def prelude-file "neo-prelude.jank")
(def prelude (binding [parse.binding/*input-file* prelude-file
                       parse.binding/*input-source* (-> (io/resource prelude-file)
                                                        slurp)]
               (parse [])))

(comment
  (binding [parse.binding/*input-file* "repl"
            parse.binding/*input-source* #_"; foo( bar spam\nfoo ; bar spa(m"
            (slurp "dev/resources/test/neo-parse/comment/single-line/pass-nested.jank")
            #_(-> (io/resource prelude-file)
                  slurp)]
    (parser parse.binding/*input-source*)))
