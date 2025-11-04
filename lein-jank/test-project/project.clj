(defproject test-project "0.1-SNAPSHOT"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "0.2"]]
  :middleware [leiningen.jank/middleware]
  :main test-project.main
  :profiles {:debug {:jank {:optimization-level 0}}
             :release {:jank {:optimization-level 2}}})
