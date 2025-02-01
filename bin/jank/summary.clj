(ns jank.summary)

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
  (append-line (str "* " s)))

(defn shell [success? cmd out]
  (append-line "\n<details>")
  (append-line (str "<summary>" (if success? "✓" "❌") " " cmd "</summary>\n"))
  (append-line "```bash")
  (append-line out)
  (append-line "```")
  (append-line "</details>\n"))
