(defproject com.jeaye/idiolisp "0.1.0-SNAPSHOT"
  :description "A statically typed functional programming language"
  :url "https://github.com/jeaye/idiolisp"
  :license {:name "idiolisp license"
            :url "https://upload.jeaye.com/idiolisp-license"}
  :plugins [[lein-cloverage "1.1.1"]
            [io.taylorwood/lein-native-image "0.3.0"]]
  :dependencies [[org.clojure/clojure "1.10.0"]
                 [org.clojure/spec.alpha "0.2.176"]
                 [org.clojure/test.check "0.9.0"]
                 [instaparse "1.4.10"]
                 [orchestra "2019.02.06-1"]
                 [expound "0.7.2"]

                 ; Colored output
                 [io.aviso/pretty "0.1.37"]
                 [venantius/glow "0.1.5"]]
  :main ^:skip-aot com.jeaye.idiolisp.core
  :target-path "target/%s"
  :native-image {:name "idiolisp"
                 :opts ["--verbose"
                        "--no-server"
                        "-H:+ReportUnsupportedElementsAtRuntime"]
                 :graal-bin "/usr/lib/jvm/java-8-graal/bin"}
  :resource-paths ["resources/"]
  :profiles {:uberjar {:aot :all
                       :main com.jeaye.idiolisp.core}
             :dev {:dependencies [[me.raynes/fs "1.4.6"]]
                   :plugins [[com.jakemccrary/lein-test-refresh "0.24.1"]]
                   :source-paths ["dev/src/"]
                   :resource-paths ["dev/resources/"]
                   :main com.jeaye.idiolisp.dev}})
