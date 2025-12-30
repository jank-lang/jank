(defproject {{raw-name}} "0.1-SNAPSHOT"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "0.3"]]
  :middleware [leiningen.jank/middleware]
  :main {{namespace}}
  :profiles {:debug {:jank {:optimization-level 0}}
             :release {:jank {:optimization-level 2}}})
