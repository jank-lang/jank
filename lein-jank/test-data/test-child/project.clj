(defproject org.jank-lang/test-child "0.1.0"
  :description "FIXME: write description"
  :url "https://example.com/FIXME"
  :license {:name "MPL 2.0"
            :url  "https://www.mozilla.org/en-US/MPL/2.0/"}
  :plugins [[org.jank-lang/lein-jank "0.7"]]
  :middleware [leiningen.jank/middleware]
  :dependencies [[org.jank-lang/test-grandchild "0.1.0"]
                 [org.jank-lang/test-grandchild2 "0.1.0"]])
