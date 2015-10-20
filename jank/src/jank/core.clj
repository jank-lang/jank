(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def as-and-bs
  (insta/parser
    "S = AB*
     AB = A B
     A = 'a'+
     B = 'b'+"))

(defn -main
  [& args]
  (println (as-and-bs "aabb")))
