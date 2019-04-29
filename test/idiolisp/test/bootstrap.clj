(ns idiolisp.test.bootstrap
  (:require [clojure.test :refer :all]
            [idiolisp.parse :as parse]
            [idiolisp.type.check :as type]
            [idiolisp.interpret.macro :as interpret]
            [idiolisp.interpret.scope.prelude :as interpret.scope.prelude]
            [idiolisp.codegen.c++ :as c++]
            [me.raynes.fs :as fs])
  (:use idiolisp.debug.log))

(defmacro consume-output
  [& body]
  `(let [s# (new java.io.StringWriter)]
     (binding [*out* s# *err* s#]
       ~@body)))

(defn slurp-resource [file]
  (slurp (clojure.java.io/resource file)))

(defn files
  [path excludes]
  (let [all (map #(.getPath %) (fs/find-files path #".*\.idiolisp"))
        matched (filter (fn [f]
                          (not-any? #(re-matches % f) excludes))
                        all)
        stripped (map #(-> (re-matches #".*/dev/resources/(.+)" %)
                           second)
                      matched)]
    stripped))

(defn try-parse [file]
  (parse/parse (slurp-resource file)))

(defn should-fail? [file]
  (some? (re-matches #".*/fail-.*" file)))

(defn valid-parse? [file]
  (consume-output (try-parse file))
  true)

(defn try-type-check [file]
  (type/check {:cells (try-parse file)}))

(defn valid-type? [file]
  (consume-output (try-type-check file))
  true)

(defn try-interpret [file]
  (let [checked (try-type-check file)]
      (interpret/evaluate (interpret.scope.prelude/create type/check)
                          (:cells checked)
                          (:scope checked))))

(defn valid-interpret? [file]
  (consume-output (try-interpret file))
  true)
