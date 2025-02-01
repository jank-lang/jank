#!/usr/bin/env bb

(ns jank.compiler+runtime.validate-formatting
  (:require
   [babashka.fs :as b.f]
   [babashka.process :as b.p]
   [clojure.string]
   [jank.util :as util]))

(def compiler+runtime-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../.."))))

(defn find-standard-clj []
  (loop [names ["standard-clj"]]
    (if (empty? names)
      (do
        (util/log-error "Unable to find a suitable standard-clj")
        (System/exit 1))
      (let [found (b.f/which (first names))]
        (if (some? found)
          (clojure.string/trim (str found))
          (recur (rest names)))))))

(defn validate-cpp-file [clang-format path results]
  (let [res (b.p/shell {:out :string
                        :err :string
                        :continue true}
                       (str clang-format " --dry-run --Werror " path))]
    (vswap! results update :count inc)
    (when-not (empty? (:err res))
      (util/log (:err res))
      (vswap! results update :errors conj {:path path}))))

(defn validate-cpp-files []
  (let [clang-format (util/find-llvm-tool "clang-format")
        results (volatile! {:count 0
                            :errors []})]
    (util/log-info "Using " clang-format)
    (doseq [dir ["include/cpp" "src/cpp" "test/cpp"]]
      (b.f/walk-file-tree (str compiler+runtime-dir "/" dir)
                          {:visit-file (fn [path _attr]
                                         (validate-cpp-file clang-format path results)
                                         :continue)}))
    (when-not (empty? (:errors @results))
      (System/exit 1))
    @results))

(defn validate-jank-files []
  (let [standard-clj (find-standard-clj)
        results (volatile! {:count 0
                            :errors []})]
    (util/log-info "Using " standard-clj)
    (util/quiet-shell {} (str standard-clj " check"))
    @results))

(defn -main [{:keys [enabled?]}]
  (util/log-step "Validate formatting")
  (if-not enabled?
    (util/log-info "Not enabled")
    (let [total-count (volatile! 0)]
      (util/with-elapsed-time duration
        (let [cpp-results (validate-cpp-files)
              jank-results (validate-jank-files)]
          (vreset! total-count (apply + (map :count [cpp-results jank-results]))))
        (util/log-info-with-time duration @total-count " files validated")))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true}))
