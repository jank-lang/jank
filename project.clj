(defproject com.jeaye/jank "0.1.0-SNAPSHOT"
  :description "A statically typed functional programming language"
  :url "https://github.com/jeaye/jank"
  :license {:name "jank license"
            :url "https://upload.jeaye.com/jank-license"}
  :aliases {"kaocha" ["run" "-m" "kaocha.runner" "--watch"]}
  :plugins [[com.jakemccrary/lein-test-refresh "0.25.0"]
            [lein-cloverage "1.2.2"]]
  :dependencies [[org.clojure/clojure "1.10.3"]
                 [org.clojure/spec.alpha "0.3.214"]
                 [org.clojure/test.check "1.1.0"]
                 [org.clojure/core.logic "1.0.0"]
                 [instaparse "1.4.10"]
                 [orchestra "2021.01.01-1"]
                 [expound "0.8.10"]

                 ; Colored output
                 [io.aviso/pretty "1.1"]
                 [venantius/glow "0.1.6"]]
  :main ^:skip-aot com.jeaye.jank.core
  :target-path "target/%s"
  :resource-paths ["resources/"]
  :profiles {:uberjar {:aot :all
                       :main com.jeaye.jank.core}
             :dev {:dependencies [[lambdaisland/kaocha "1.60.945"]
                                  [me.raynes/fs "1.4.6"]]
                   :source-paths ["dev/src/"]
                   :resource-paths ["dev/resources/"]
                   :main com.jeaye.jank.dev}})
