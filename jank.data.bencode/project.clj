(defproject org.jank-lang/data.bencode "0.1.0-SNAPSHOT"
  :license {:name "MPL 2.0"
            :url "https://www.mozilla.org/en-US/MPL/2.0/"}
  :dependencies [[org.clojure/clojure "1.11.1"]]
  :plugins [[org.jank-lang/lein-jank "0.0.1-SNAPSHOT"]]
  :main ^:skip-aot jank.data.bencode
  :target-path "target/%s"
  :jank {:include-paths []
         :includes []}
  :source-paths ["src/jank"
                 "src/cpp"]
  :profiles {:uberjar {:aot :all
                       :jvm-opts ["-Dclojure.compiler.direct-linking=true"]}})
