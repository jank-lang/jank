#!/usr/bin/env bb

(ns jank.compiler+runtime.core
  (:require [jank.util :as util]
            [jank.compiler+runtime.validate-formatting]
            [jank.compiler+runtime.build+test]
            [jank.compiler+runtime.bash-test]
            [jank.compiler+runtime.coverage]
            [jank.compiler+runtime.package]))

(defn -main [{:keys [validate-formatting? build?]}]
  (util/log-boundary "compiler+runtime")

  ; Formatting
  ; TODO: Have a reusable fn here for other projects.
  (jank.compiler+runtime.validate-formatting/-main {:enabled? validate-formatting?})

  ; Compile and test
  (jank.compiler+runtime.build+test/-main {:enabled? build?
                                           :build-type (util/get-env "JANK_BUILD_TYPE" "Debug")
                                           :analyze (util/get-env "JANK_ANALYZE" "off")
                                           :sanitize (util/get-env "JANK_SANITIZE" "none")
                                           :coverage (util/get-env "JANK_COVERAGE" "off")})

  ; Bash tests
  (jank.compiler+runtime.bash-test/-main {:enabled? (and build?
                                                         ; AOT compilation fails when sanitizers are enabled.
                                                         (= "none" (util/get-env "JANK_SANITIZE" "none")))})

  ; Codecov (merge results)
  (jank.compiler+runtime.coverage/-main {:enabled? (= "on" (util/get-env "JANK_COVERAGE" "off"))})

  ; Distro packaging
  (jank.compiler+runtime.package/-main {:enabled? (= "on" (util/get-env "JANK_PACKAGE" "off"))}))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:validate-formatting? true
          :build? true}))
