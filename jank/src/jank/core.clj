(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def grammar
  (insta/parser
    (clojure.java.io/resource "grammar")))

(defn -main
  [& args]
  (println
    (grammar "
(print 42)
(print 3.14)
(print \"this is a string\")
")))
