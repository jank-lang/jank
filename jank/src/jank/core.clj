(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def parse
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn handle [current ast]
  (println current)
  ast
  )

(defn -main
  [& args]
  (let [parsed (parse (slurp (first args)))]
    (when parsed
      (loop [current (first parsed) remaining (rest parsed)
             ast '[]]
        (cond
          (nil? current) ast
          :else (recur (first remaining)
                       (rest remaining)
                       (handle current ast))
        )))))
