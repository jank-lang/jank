(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def grammar
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn -main
  [& args]
  (println
    (grammar "
(bind kitty \"meow\")
(bind four (+ 2 2))
(ƒ ident (s string) (string)
  s)
")))
