(ns com.jeaye.jank.test.bootstrap
  (:require [clojure.spec.alpha :as s]
            [orchestra.spec.test :as stest]
            [expound.alpha :as expound]
            [me.raynes.fs :as fs]
            [com.jeaye.jank.parse :as parse]
            [com.jeaye.jank.parse.binding :as parse.binding]))

(defmacro with-consumed-output
  [& body]
  `(let [s# (new java.io.StringWriter)]
     (binding [*out* s#
               *err* s#]
       ~@body)))

(defn with-instrumentation [fun]
  (s/check-asserts true)
  (->> (constantly (expound/custom-printer {:show-valid-values? true}))
       (alter-var-root #'s/*explain-out*))
  (stest/instrument)
  (fun)
  (stest/unstrument))

(defn slurp-resource [file]
  (-> file
      clojure.java.io/resource
      slurp))

(defn files
  [path excludes]
  (let [all (map #(.getPath %) (fs/find-files path #".*\.jank"))
        dev-resources-regex #".*/dev/resources/(.+)"]
    (->> all
         (map (fn [file]
                {:resource (-> (re-matches dev-resources-regex
                                           file)
                               second)
                 :skip? (some #(re-matches % file) excludes)})))))

(defn try-parse [file]
  (binding [parse.binding/*input-file* file
            parse.binding/*input-source* (slurp-resource file)]
    (parse/parse parse/prelude)))

(defn should-fail? [file-info]
  (some? (re-matches #".*/fail-.*" (:resource file-info))))

(defn valid-parse? [file-info]
  (with-consumed-output
    (try-parse (:resource file-info)))
  true)
