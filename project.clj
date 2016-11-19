(defproject jank "0.1.0-SNAPSHOT"
  :description "A statically typed functional programming language"
  :url "https://github.com/jeaye/jank"
  :license {:name "jank license"
            :url "https://upload.jeaye.com/jank-license"}
  :dependencies [[org.clojure/clojure "1.9.0-alpha13"]
                 [instaparse "1.4.3"]
                 [org.clojure/core.logic "0.8.10"]
                 [me.raynes/fs "1.4.6"]]
  :main ^:skip-aot jank.core
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all}
             :dev {:source-paths ["dev/"]
                   :main jank.dev}})
