(ns jank.summary
  (:require [clojure.string]))

(defonce contents (atom ""))

(defn summary-file []
  (System/getenv "GITHUB_STEP_SUMMARY"))

(defn enabled? []
  (some? (summary-file)))

(defn initialize []
  (when (enabled?)
    (-> (Runtime/getRuntime)
        (.addShutdownHook (Thread. (fn []
                                     (spit (summary-file) @contents)))))))

(defn append-line [s]
  (swap! contents #(str % s "\n")))

(defn boundary [title]
  (append-line (str "## " title)))

(defn step [title]
  (append-line (str "### " title)))

(defn log [s]
  (append-line (str s "\n")))

(defn strip-ansi-codes
  [s]
  (clojure.string/replace s #"\x1B\[[0-9;]*[mK]" ""))

(defn shell [success? cmd out]
  (append-line "\n<details>")
  (append-line (str "<summary>" (if success? "✓" "❌") " " cmd "</summary>\n"))
  (append-line "```text")
  (append-line (strip-ansi-codes out))
  (append-line "```")
  (append-line "</details>\n"))
