(ns jank.core
  (:gen-class)
  (:require [instaparse.core :as insta]))

(def parse
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))

(defmulti handle
  (fn [current ast]
    (first current)))

(defmethod handle :lambda-definition [current ast]
  (conj (:cells ast) current))

(defmethod handle :macro-definition [current ast]
  ast)

(defmethod handle :binding-definition [current ast]
  ast)

(defmethod handle :function-call [current ast]
  ast)

(defmethod handle :default [current ast]
  (assert false (str "invalid handler for " current)))

(defn empty-ast []
  {:cells []})

(defn -main
  [& args]
  (let [parsed (parse (slurp (first args)))]
    (when parsed
      (loop [current (first parsed) remaining (rest parsed)
             ast (empty-ast)]
        (println current)
        (cond
          (nil? current) ast
          :else (recur (first remaining)
                       (rest remaining)
                       (handle current ast))
        )))))
