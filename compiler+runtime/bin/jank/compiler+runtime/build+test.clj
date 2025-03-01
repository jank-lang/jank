#!/usr/bin/env bb

(ns jank.compiler+runtime.build+test
  (:require [clojure.string]
            [babashka.fs :as b.f]
            [jank.util :as util]))

(def compiler+runtime-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../.."))))

(defn -main [{:keys [enabled?
                     build-type
                     analyze
                     sanitize
                     coverage]}]
  (util/log-step "Compile and test")
  (if-not enabled?
    (util/log-info "Not enabled")
    (let [clang (util/find-llvm-tool "clang")
          clang++ (util/find-llvm-tool "clang++")
          exports (merge {"CC" clang
                          "CXX" clang++
                          "CCACHE_BASEDIR" compiler+runtime-dir
                          "CCACHE_DIR" (str compiler+runtime-dir "/.ccache")
                          "CCACHE_COMPRESS" "true"
                          "CCACHE_MAXSIZE" "1G"
                          "CLANG_TIDY_CACHE_DIR" (str compiler+runtime-dir "/.ctcache")}
                         (when (= "on" analyze)
                           {"CMAKE_CXX_CLANG_TIDY" "clang-tidy-cache"}))
          configure-flags ["-GNinja"
                           "-Djank_tests=on"
                           "-DCMAKE_C_COMPILER_LAUNCHER=ccache"
                           "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
                           (str "-DCMAKE_BUILD_TYPE=" build-type)
                           (str "-Djank_analyze=" analyze)
                           (str "-Djank_sanitize=" sanitize)
                           (str "-Djank_coverage=" coverage)]
          configure-cmd (str "./bin/configure " (clojure.string/join " " configure-flags))]
      (util/quiet-shell {:dir compiler+runtime-dir
                         :extra-env exports}
                        "ccache --zero-stats")

      (util/quiet-shell {:dir compiler+runtime-dir
                         :extra-env exports}
                        configure-cmd)
      (util/log-info "Configured")

      (util/with-elapsed-time duration
        (util/quiet-shell {:dir compiler+runtime-dir
                           :extra-env exports}
                          "./bin/compile")
        (util/log-info-with-time duration "Compiled"))

      (util/quiet-shell {:dir compiler+runtime-dir
                         :extra-env exports}
                        "ccache --show-stats")

      (util/with-elapsed-time duration
        (util/quiet-shell {:dir compiler+runtime-dir
                           :extra-env (merge exports
                                             {"LLVM_PROFILE_FILE" (str compiler+runtime-dir "/build/jank-%p.profraw")})}
                          "./bin/test")
        (util/log-info-with-time duration "Tested")))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true
          :build-type "Debug"
          :analyze "off"
          :sanitize "none"
          :coverage "off"}))
