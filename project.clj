(defproject lein-jank "0.0.1-SNAPSHOT"
  :description "Build your jank projects using leiningen"
  :url "https://jank-lang.org/"
  :license {:name "EPL-2.0 OR GPL-2.0-or-later WITH Classpath-exception-2.0"
            :url "https://www.eclipse.org/legal/epl-2.0/"}
  :dependencies [[babashka/process "0.5.21"]
                 [leiningen-core "2.10.0"]]
  :eval-in-leiningen true)
