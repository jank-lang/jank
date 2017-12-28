(defproject com.jeaye/jank "0.1.0-SNAPSHOT"
  :description "A statically typed functional programming language"
  :url "https://github.com/jeaye/jank"
  :license {:name "jank license"
            :url "https://upload.jeaye.com/jank-license"}
  :plugins [[lein-cloverage "1.0.10"]]
  :dependencies [[org.clojure/clojure "1.9.0"]
                 [org.clojure/core.logic "0.8.11"]
                 [instaparse "1.4.8"]
                 [me.raynes/fs "1.4.6"]
                 [io.aviso/pretty "0.1.34"]]
  :main ^:skip-aot jank.core
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all}
             :dev {:source-paths ["dev/" "playground/"]
                   :main jank.dev}
             :coverage {:resource-paths ["dev-resources/"]}
             :benchmark {:dependencies [[criterium "0.4.4"]
                                        [clj-time "0.14.2"]]
                         :source-paths ["benchmark/"]
                         :resource-paths ["dev-resources/"]
                         :main jank.benchmark}})
