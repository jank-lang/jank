(ns jank.util
  (:require [babashka.fs :as b.f]
            [babashka.process :as b.p]
            [clojure.string]
            [jank.summary :as summary]))

(def compiler+runtime-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../compiler+runtime"))))
(def llvm-bin-dir (str compiler+runtime-dir "/build/llvm-install/usr/local/bin"))

(def llvm-version 22)

(defn get-env
  ([s]
   (get-env s nil))
  ([s fallback]
   (let [raw (System/getenv s)]
     (if-not (empty? raw)
       raw
       fallback))))

(defmacro string=
  "Compare L and R strings ignoring line ending differences."
  [l r]
  `(= (string/split-lines ~l) (string/split-lines ~r)))

(defn format-ms [ms]
  (let [units [[3600000 "h"] [60000 "m"] [1000 "s"] [1 "ms"]]
        extract (fn [ms [unit label]]
                  (let [amt (quot ms unit)]
                    (if (pos? amt)
                      [(mod ms unit) [amt label]]
                      [ms nil])))]
    (loop [ms ms, units units, result []]
      (if (or (empty? units) (>= (count result) 2))
        (clojure.string/join " " (map (fn [[v l]] (str v l)) result))
        (let [[rem-ms val] (extract ms (first units))]
          (recur rem-ms (rest units) (if val (conj result val) result)))))))

(defn log-boundary [title]
  (println "\n────────────────" title "────────────────")
  (summary/boundary title))

(defn log-step [title]
  (println "\n────" title "────")
  (summary/step title))

(defn log [& args]
  (let [s (clojure.string/join " " args)]
    (println s)
    (summary/log s)))

(defn log-info [& args]
  (log "🛈 " (apply str args)))

(defn log-info-with-time [time-ms & args]
  (log "🛈 " (apply str args) (str "(" (format-ms time-ms) ")")))

(defn log-warning [& args]
  (log "⚠ " (apply str args)))

(defn log-warning-with-time [time-ms & args]
  (log "⚠ " (apply str args) (str "(" (format-ms time-ms) ")")))

(defn log-error [& args]
  (log "❌ " (apply str args)))

(defn log-error-with-time [time-ms & args]
  (log "❌ " (apply str args) (str "(" (format-ms time-ms) ")")))

(defn command-make-portable
  "Converts CMD into a form that can be executed portably across
  supported platforms."
  [cmd]
  (if (b.f/windows?)
    ;; Assumes an MSYS2 environment and runs the command via bash.
    (let [bash-exe (b.f/which "bash")]
      (when-not bash-exe
        (throw (ex-info "Unable to find bash." {})))
      (let [[prog & rest] (clojure.string/split cmd #"\s+")
            unix-prog (b.f/unixify prog)
            rebuilt (clojure.string/join " " (cons unix-prog rest))]
        (str bash-exe " -c \"" rebuilt "\"")))
    cmd))

(defn quiet-shell
  "Runs the shell string CMD in a cross-platform manner using
  `babashka.process/process` with PROPS, capturing stdout and stderr
  and producing a summary.  Returns the process on success.  On
  failure, prints failed output, and exits the program with code 1."
  [props cmd]
  ;; (println :props props :cmd cmd)
  (let [cmd (command-make-portable cmd)
        proc @(b.p/process
               (merge {:out :string
                       :err :out}
                      props)
               cmd)]
    (if-not (zero? (:exit proc))
      (do
        (log-error "Failed to run command " cmd)
        (println (:out proc))
        (summary/shell false props cmd (:out proc))
        (System/exit 1))
      (do
        (summary/shell true props cmd (:out proc))
        proc))))

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
  (loop [names [(str llvm-bin-dir "/" tool) (str tool "-" llvm-version) tool]]
    (if (empty? names)
      (do
        (log-error "Unable to find a suitable " tool " for LLVM " llvm-version)
        (System/exit 1))
      (let [found (b.f/which (first names))]
        (if (and (some? found) (= (str llvm-version)
                                  (extract-llvm-tool-format-version found)))
          (clojure.string/trim (str found))
          (recur (rest names)))))))
