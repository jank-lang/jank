(ns com.jeaye.jank.test.bootstrap
  (:require [clojure.spec.alpha :as s]
            [clojure.java.io]
            [clojure.test :refer [is]]
            [orchestra.spec.test :as stest]
            [expound.alpha :as expound]
            [me.raynes.fs :as fs]
            [com.jeaye.jank.parse :as parse]
            [com.jeaye.jank.parse.binding :as parse.binding]
            [com.jeaye.jank.semantic.core :as semantic.core]))

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

(defmacro thrown-with-dump? [prefix fun]
  `(try
     ~fun
     false
     (catch Exception e#
       (is (= (.getMessage e#) ~prefix))
       true)))

(defn files
  [path excludes]
  (let [all (map #(.getPath %) (fs/find-files path #".*\.jank"))
        dev-resources-regex #".*/dev/resources/(.+)"]
    (->> all
         (map (fn [file]
                {:resource (second (re-matches dev-resources-regex file))
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

(defn try-semantic-check [file]
  (-> (try-parse file)
      (semantic.core/pass-1 {} [])))

(defn valid-semantic-check? [file-info]
  (with-consumed-output
    (try-semantic-check (:resource file-info)))
  true)

(defn test-files [tag error-regex try-fn path excludes]
  (println tag "gathering files...")
  (let [files (files path excludes)]
    (doseq [file files]
      (if (:skip? file)
        (println tag "skip" (:resource file))
        (do
          (println tag "testing" (:resource file))
          (if (should-fail? file)
            (is (thrown-with-dump? error-regex (try-fn file)))
            (is (try-fn file))))))))
