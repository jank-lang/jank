(ns jank.test.bootstrap
  (:require [clojure.test :refer :all]
            [jank.parse :as parse]
            [jank.type.check :as type]
            [jank.codegen.c++ :as c++]
            [me.raynes.fs :as fs])
  (:use jank.debug.log))

(defmacro consume-output
  [& body]
  `(let [s# (new java.io.StringWriter)]
     (binding [*out* s# *err* s#]
       ~@body)))

(defn slurp-resource [file]
  (slurp (clojure.java.io/resource file)))

(defn files
  [path excludes]
  (let [all (map #(.getPath %) (fs/find-files path #".*\.jank"))
        matched (filter (fn [f]
                          (not-any? #(re-matches % f) excludes))
                        all)
        stripped (map #(-> (re-matches #".*/dev-resources/(.+)" %)
                           second)
                      matched)]
    stripped))

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
