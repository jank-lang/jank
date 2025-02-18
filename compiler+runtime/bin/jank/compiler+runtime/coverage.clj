#!/usr/bin/env bb

(ns jank.compiler+runtime.coverage
  (:require [clojure.string]
            [jank.util :as util]
            [babashka.fs :as b.f]))

(def compiler+runtime-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../.."))))

(defn -main [{:keys [enabled?]}]
  (util/log-step "Upload coverage report")
  (cond
    (not enabled?)
    (util/log-info "Not enabled")

    (nil? (util/get-env "CODECOV_TOKEN"))
    (util/log-info "Skipping due to missing $CODECOV_TOKEN")

    :else
    (util/with-elapsed-time duration
      (let [llvm-profdata (util/find-llvm-tool "llvm-profdata")
            llvm-cov (util/find-llvm-tool "llvm-cov")
            coverage-files (b.f/glob (str compiler+runtime-dir "/build")
                                     "jank-*.profraw"
                                     {:recursive false})
            merged-file (str compiler+runtime-dir "/build/jank.profdata")
            lcov-file (str compiler+runtime-dir "/build/jank.lcov")]
        (util/quiet-shell {:dir compiler+runtime-dir}
                          (str llvm-profdata " merge "
                               (clojure.string/join " " coverage-files)
                               " -o " merged-file))
        (util/quiet-shell {:dir compiler+runtime-dir
                           :out lcov-file}
                          (str llvm-cov " export --format=lcov --instr-profile " merged-file
                               " build/jank-test --object build/jank"))
        (let [codecov-script (str compiler+runtime-dir "/build/codecov")
              sha (or (util/get-env "GITHUB_SHA")
                      (util/quiet-shell {} "git rev-parse HEAD"))]
          (util/quiet-shell {:out codecov-script} "curl -s https://cli.codecov.io/latest/linux/codecov")
          (util/quiet-shell {} (str "chmod +x " codecov-script))
          (util/quiet-shell {}
                            (str codecov-script
                                 " upload-process --disable-search --fail-on-error"
                                 " -C " sha
                                 " -t " (util/get-env "CODECOV_TOKEN")
                                 " -n " (util/get-env "JANK_MATRIX_ID" "matrix") "-" (util/get-env "GITHUB_RUN_ID" "local")
                                 " -f " lcov-file))))
      (util/log-info-with-time duration "Merged and published coverage report"))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true}))
