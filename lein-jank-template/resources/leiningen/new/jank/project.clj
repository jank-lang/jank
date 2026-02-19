(defproject {{raw-name}} "0.1-SNAPSHOT"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "0.5"]]
  :middleware [leiningen.jank/middleware]
  :main {{namespace}}
  :profiles {:base {:jank {:output-dir "target/debug"
                           :optimization-level 0}}
             :release {:jank {:output-dir "target/release"
                              :optimization-level 2}}})
