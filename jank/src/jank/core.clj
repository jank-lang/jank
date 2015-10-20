(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def as-and-bs
  (insta/parser
    (clojure.java.io/resource "grammar")))

(defn -main
  [& args]
  (println (as-and-bs "aabb")))
