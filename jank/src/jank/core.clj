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

(defn handle-call [current ast]
  (println "handling call")
  ast)

(defn handle-list [current ast]
  (println "handling list")
  ast)

(def handlers {:function-definition handle-function
               :macro-definition handle-macro
               :binding-definition handle-binding
               :function-call handle-call
               :list handle-list})

(defn handle [current ast]
  (let [handler (handlers (first current))]
    (assert handler "invalid handler")
    (handler current ast)
  ))

(defn -main
  [& args]
  (let [parsed (parse (slurp (first args)))]
    (when parsed
      (loop [current (first parsed) remaining (rest parsed)
             ast '[]]
        (println current)
        (cond
          (nil? current) ast
          :else (recur (first remaining)
                       (rest remaining)
                       (handle current ast))
        )))))
