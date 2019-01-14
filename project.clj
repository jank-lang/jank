(defproject com.jeaye/jank "0.1.0-SNAPSHOT"
  :description "A statically typed functional programming language"
  :url "https://github.com/jeaye/jank"
  :license {:name "jank license"
            :url "https://upload.jeaye.com/jank-license"}
  :plugins [[lein-cloverage "1.0.13"]
            [io.taylorwood/lein-native-image "0.3.0"]]
  :dependencies [[org.clojure/clojure "1.10.0"]
                 [org.clojure/core.logic "0.8.11"]
                 [instaparse "1.4.10"]
                 [me.raynes/fs "1.4.6"]

                 ; Colored output
                 [io.aviso/pretty "0.1.36"]
                 [venantius/glow "0.1.5"]]
  :main ^:skip-aot com.jeaye.jank.core
  :target-path "target/%s"
  :native-image {:name "jank"
                 :opts ["--verbose"
                        "--no-server"
                        "-H:+ReportUnsupportedElementsAtRuntime"]
                 :graal-bin "/usr/lib/jvm/java-8-graal/bin"}
  :resource-paths ["resources/"]
  :profiles {:uberjar {:aot :all
                       :main com.jeaye.jank.core}
             :dev {:source-paths ["dev/src/"]
                   :resource-paths ["dev/resources/"]
                   :main com.jeaye.jank.dev}})
