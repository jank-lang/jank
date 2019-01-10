(defproject com.jeaye/jank "0.1.0-SNAPSHOT"
  :description "A statically typed functional programming language"
  :url "https://github.com/jeaye/jank"
  :license {:name "jank license"
            :url "https://upload.jeaye.com/jank-license"}
  :plugins [[lein-cloverage "1.0.10"]
            [io.taylorwood/lein-native-image "0.2.0"]]
  :dependencies [[org.clojure/clojure "1.9.0"]
                 [org.clojure/core.logic "0.8.11"]
                 [instaparse "1.4.8"]
                 [me.raynes/fs "1.4.6"]

                 ; Colored output
                 [io.aviso/pretty "0.1.34"]
                 [venantius/glow "0.1.4"]]
  :main ^:skip-aot jank.core
  :target-path "target/%s"
  :native-image {:name "jank"
                 :opts ["--verbose"
                        "--no-server"
                        "-H:+ReportUnsupportedElementsAtRuntime"]
                 :graal-bin "/usr/lib/jvm/java-8-graal/bin"}
  :profiles {:uberjar {:aot :all
                       :main com.jeaye.jank.core}
             :dev {:source-paths ["dev/src/"]
                   :main com.jeaye.jank.dev}
             :coverage {:resource-paths ["dev-resources/"]}
             :benchmark {:dependencies [[criterium "0.4.4"]
                                        [clj-time "0.14.2"]]
                         :source-paths ["benchmark/"]
                         :resource-paths ["dev-resources/"]
                         :main jank.benchmark}})
