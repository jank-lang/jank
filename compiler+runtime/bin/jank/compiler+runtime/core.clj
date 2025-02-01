#!/usr/bin/env bb

(ns jank.compiler+runtime.core
  (:require [jank.util :as util]
            [jank.compiler+runtime.validate-formatting]
            [jank.compiler+runtime.build+test]
            [jank.compiler+runtime.bash-test]
            [jank.compiler+runtime.coverage]))

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
  (jank.compiler+runtime.bash-test/-main {:enabled? build?})

  ; Codecov (merge results)
  (jank.compiler+runtime.coverage/-main {:enabled? (= "on" (util/get-env "JANK_COVERAGE" "off"))}))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:validate-formatting? true
          :build? true}))
