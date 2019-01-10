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
        dev-resources-regex #".*/dev/resources/(.+)"]
    (map (fn [file]
           (hash-map :resource (-> (re-matches dev-resources-regex
                                               file)
                                   second)
                     :skip? (some #(re-matches % file)
                                  excludes)))
         all)))

(defn try-parse [file]
  (binding [parse.binding/*input-file* file
            parse.binding/*input-source* (slurp-resource file)]
    (parse/parse parse/prelude)))

(defn should-fail? [file-info]
  (some? (re-matches #".*/fail-.*" (:resource file-info))))

(defn valid-parse? [file-info]
  (consume-output (try-parse (:resource file-info)))
  true)
