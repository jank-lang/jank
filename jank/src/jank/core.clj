(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def parse
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn handle-function [current ast]
  (println "handling function")
  ast)

(defn handle-macro [current ast]
  (println "handling macro")
  ast)

(defn handle-binding [current ast]
  (println "handling binding")
  ast)

(defn handle-list [current ast]
  (println "handling list")
  ast)

(def handlers {:function-definition handle-function
               :macro-definition handle-macro
               :binding-definition handle-binding
               :list handle-list})

(defn handle [current ast]
  ((handlers (first current)) current ast)
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
