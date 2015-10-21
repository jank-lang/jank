(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def parse
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defn handle-function [handlers current ast]
  (println "handling function")
  ast)

(defn handle-macro [handlers current ast]
  (println "handling macro")
  ast)

(defn handle-binding [handlers current ast]
  (println "handling binding")
  ast)

(defn handle-call [handlers current ast]
  (println "handling call")
  ast)

(def handlers {:function-definition handle-function
               :macro-definition handle-macro
               :binding-definition handle-binding
               :function-call handle-call})

(defn handle [current ast]
  (let [handler (handlers (first current))]
    (assert handler (str "invalid handler for " current))
    (handler handlers current ast)
  ))

(defn -main
  [& args]
  (let [parsed (parse (slurp (first args)))]
    (when parsed
      (loop [current (first parsed) remaining (rest parsed)
             ast {}]
        (println current)
        (cond
          (nil? current) ast
          :else (recur (first remaining)
                       (rest remaining)
                       (handle current ast))
        )))))
