(defproject jank "0.1.0-SNAPSHOT"
  :description "FIXME: write description"
  :url "http://example.com/FIXME"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :dependencies [[org.clojure/clojure "1.7.0"]
                 [instaparse "1.4.1"]]
  :main ^:skip-aot jank.core
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all}})
