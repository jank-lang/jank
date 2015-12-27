(ns jank.bootstrap
  (:require [clojure.test :refer :all]
            [jank.parse :as parse]
            [jank.type.check :as type]
            [jank.codegen.c++ :as c++]))

(defmacro consume-output
  [& body]
  `(let [s# (new java.io.StringWriter)]
     (binding [*out* s# *err* s#]
       ~@body)))

(defn slurp-resource [file]
  (slurp (clojure.java.io/resource file)))

(defn parse [file]
  (consume-output
    (parse/parse (slurp-resource file))))

(defn should-fail? [file]
  (some? (re-matches #".*/fail-.*" file)))

(defn valid-parse? [file]
  (parse file)
  true)

(defn valid-type? [file]
  (consume-output
    (type/check {:cells (parse file)}))
  true)

(defn valid-codegen? [file]
  (consume-output
    (c++/codegen (first (type/check {:cells (parse file)}))))
  true)
