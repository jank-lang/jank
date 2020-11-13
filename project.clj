(defproject com.jeaye/jank "0.1.0-SNAPSHOT"
  :description "A statically typed functional programming language"
  :url "https://github.com/jeaye/jank"
  :license {:name "jank license"
            :url "https://upload.jeaye.com/jank-license"}
  :plugins [[lein-cloverage "1.1.1"]]
  :dependencies [[org.clojure/clojure "1.10.0"]
                 [org.clojure/spec.alpha "0.2.176"]
                 [org.clojure/test.check "0.9.0"]
                 [instaparse "1.4.10"]
                 [orchestra "2019.02.06-1"]
                 [expound "0.7.2"]

                 ; Colored output
                 [io.aviso/pretty "0.1.37"]
                 [venantius/glow "0.1.5"]]
  :main ^:skip-aot com.jeaye.jank.core
  :target-path "target/%s"
  :resource-paths ["resources/"]
  :profiles {:uberjar {:aot :all
                       :main com.jeaye.jank.core}
             :dev {:dependencies [[lambdaisland/kaocha "1.0.641"]
                                  [me.raynes/fs "1.4.6"]]
                   :source-paths ["dev/src/"]
                   :resource-paths ["dev/resources/"]
                   :main com.jeaye.jank.dev}})
