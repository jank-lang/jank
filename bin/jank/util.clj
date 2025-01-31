(ns jank.util
  (:require
   [babashka.fs :as b.f]
   [babashka.process :as b.p]
   [clojure.string]))

(def llvm-version 19)

(defn log [& args]
  (println (apply str args)))

(defn log-boundary [title]
  (log "\n──────────────── " title " ────────────────"))

(defn log-step [title]
  (log "\n──── " title " ────"))

(defn log-info [& args]
  (println "🛈 " (apply str args)))

(defn log-info-with-time [time-ms & args]
  ; TODO: Time formatting.
  (println "🛈 " (apply str args) (str "(" time-ms " ms)")))

(defn log-warning [& args]
  (println "⚠ " (apply str args)))

(defn log-error [& args]
  (println "❌ " (apply str args)))

(defn log-error-with-time [time-ms & args]
  ; TODO: Time formatting.
  (println "❌ " (apply str args) (str "(" time-ms " ms)")))

(defn quiet-shell [props cmd]
  (let [proc @(b.p/process
                (merge {:out :string
                        :err :out}
                       props)
                cmd)]
    (if-not (zero? (:exit proc))
      (do
        (log-error "Failed to run command " cmd)
        (log (:out proc))
        (System/exit 1))
      proc)))

(defmacro with-elapsed-time
  [time-sym expr-to-time expr-with-time-sym]
  `(let [start# (. System (nanoTime))
         ret# ~expr-to-time
         ~time-sym (long (/ (double (- (. System (nanoTime)) start#)) 1000000.0))]
     ~expr-with-time-sym))

(defn extract-llvm-tool-format-version [tool]
  (let [res (b.p/shell {:out :string
                        :err :string}
                       (str tool " --version"))
        major-version (->> (clojure.string/replace (clojure.string/trim (:out res)) #"\n" " ")
                           (re-matches #".*version (\d+).*")
                           second)]
    major-version))

(defn find-llvm-tool [tool]
  (loop [names [(str tool "-" llvm-version) tool]]
    (if (empty? names)
      (do
        (log-error "Unable to find a suitable " tool " for LLVM " llvm-version)
        (System/exit 1))
      (let [found (b.f/which (first names))]
        (if (and (some? found) (= (str llvm-version)
                                  (extract-llvm-tool-format-version found)))
          (clojure.string/trim (str found))
          (recur (rest names)))))))
