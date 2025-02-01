#!/usr/bin/env bb

(ns jank.compiler+runtime.build+test
  (:require
   [babashka.fs :as b.f]
   [clojure.string]
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
          exports {"CC" clang
                   "CXX" clang++}
          configure-flags ["-GNinja"
                           "-Djank_tests=on"
                           (str "-DCMAKE_BUILD_TYPE=" build-type)
                           (str "-Djank_analysis=" analyze)
                           (str "-Djank_sanitize=" sanitize)
                           (str "-Djank_coverage=" coverage)]
          configure-cmd (str "./bin/configure " (clojure.string/join " " configure-flags))]
      (util/quiet-shell {:dir compiler+runtime-dir
                         :extra-env exports}
                        configure-cmd)
      (util/log-info "Configured")

      (util/with-elapsed-time duration
        (util/quiet-shell {:dir compiler+runtime-dir
                           :extra-env exports}
                          "./bin/compile")
        (util/log-info-with-time duration "Compiled"))

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
