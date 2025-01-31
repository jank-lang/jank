#!/usr/bin/env bb

(ns jank.compiler+runtime.core
  (:require
   [jank.compiler+runtime.bash-test]
   [jank.compiler+runtime.build+test]
   [jank.compiler+runtime.coverage]
   [jank.compiler+runtime.validate-formatting]
   [jank.util :as util]))

(defn -main [{:keys [validate-formatting? build?]}]
  (util/log-boundary "Test compiler+runtime")

  ; Formatting
  (jank.compiler+runtime.validate-formatting/-main {:enabled? validate-formatting?})

  ; Compile and test
  (jank.compiler+runtime.build+test/-main {:enabled? build?
                                           :build-type (or (System/getenv "JANK_BUILD_TYPE") "Debug")
                                           :analyze (or (System/getenv "JANK_ANALYZE") "off")
                                           :sanitize (or (System/getenv "JANK_SANITIZE") "none")
                                           :coverage (or (System/getenv "JANK_COVERAGE") "off")})

  ; Bash tests
  (jank.compiler+runtime.bash-test/-main {:enabled? build?})

  ; Codecov (merge results)
  (jank.compiler+runtime.coverage/-main {:enabled? (= "on" (or (System/getenv "JANK_COVERAGE") "off"))}))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:validate-formatting? true
          :build? true}))
