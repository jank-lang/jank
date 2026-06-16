(defproject org.jank-lang/test-grandchild "0.1.0"
  :description "FIXME: write description"
  :url "https://example.com/FIXME"
  :license {:name "MPL 2.0"
            :url  "https://www.mozilla.org/en-US/MPL/2.0/"}
  :plugins [[org.jank-lang/lein-jank "0.7"]]
  :middleware [leiningen.jank/middleware]
  :build-dependencies [[org.jank-lang/test-build-dependency "0.1.0"]]
  :verbatim-paths ["jank-build.bb"])
