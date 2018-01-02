(ns com.jeaye.jank.test.bootstrap
  (:require [me.raynes.fs :as fs]
            [com.jeaye.jank.parse :as parse]
            [com.jeaye.jank.parse.binding :as parse.binding]))

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

(defn try-parse [file]
  (binding [parse.binding/*input-file* file
            parse.binding/*input-source* (slurp-resource file)]
    (parse/parse parse/prelude)))

(defn should-fail? [file]
  (some? (re-matches #".*/fail-.*" file)))

(defn valid-parse? [file]
  (consume-output (try-parse file))
  true)
