(defproject org.jank-lang/lein-jank "2026.07-1"
  :description "Build your jank projects using Leiningen."
  :url "https://jank-lang.org/"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies [[babashka/fs "0.5.33"]
                 [babashka/process "0.6.25"]
                 [leiningen-core "2.12.0"]
                 [org.clojure/tools.cli "1.4.256"]
                 [org.clojure/tools.namespace "1.5.1"]]
  :profiles {:dev {:dependencies [[nubank/matcher-combinators "3.10.0"]]}}
  :eval-in-leiningen true)
