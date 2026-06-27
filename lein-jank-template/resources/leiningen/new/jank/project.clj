(defproject {{raw-name}} "0.1-SNAPSHOT"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "2026.06-2"]]
  :middleware [leiningen.jank/middleware]
  :main {{namespace}}
  :profiles {:base {:jank {:target-dir "target/debug"
                           :optimization-level 0}}
             :release {:jank {:target-dir "target/release"
                              :optimization-level 3}}})
