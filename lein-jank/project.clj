(defproject org.jank-lang/lein-jank "0.0.1-SNAPSHOT"
  :description "Build your jank projects using leiningen."
  :url "https://jank-lang.org/"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies [[babashka/fs "0.5.20"]
                 [babashka/process "0.5.22"]
                 [leiningen-core "2.11.2"]]
  :eval-in-leiningen true)
