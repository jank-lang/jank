(ns com.jeaye.idiolisp.test.bootstrap
  (:require [clojure.spec.alpha :as s]
            [orchestra.spec.test :as stest]
            [expound.alpha :as expound]
            [me.raynes.fs :as fs]
            [com.jeaye.idiolisp.parse :as parse]
            [com.jeaye.idiolisp.parse.binding :as parse.binding]))

(defmacro consume-output
  [& body]
  `(let [s# (new java.io.StringWriter)]
     (binding [*out* s# *err* s#]
       ~@body)))

(defn with-instrumentation [fun]
  (s/check-asserts true)
  (stest/instrument)
  (set! s/*explain-out* (expound/custom-printer {:show-valid-values? true}))
  (fun))

(defn slurp-resource [file]
  (-> file
      clojure.java.io/resource
      slurp))

(defn files
  [path excludes]
  (let [all (map #(.getPath %) (fs/find-files path #".*\.io"))
        dev-resources-regex #".*/dev/resources/(.+)"]
    (map (fn [file]
           {:resource (-> (re-matches dev-resources-regex
                                      file)
                          second)
            :skip? (some #(re-matches % file) excludes)})
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
