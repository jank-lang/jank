(defproject lein-jank "0.0.1-SNAPSHOT"
  :description "Build your jank projects using leiningen."
  :url "https://jank-lang.org/"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies [[babashka/fs "0.4.9"]
                 [babashka/process "0.5.21"]
                 [leiningen-core "2.10.0"]]
  :eval-in-leiningen true)
