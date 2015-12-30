(defproject jank "0.1.0"
  :description "A statically typed functional programming language"
  :url "https://github.com/jeaye/jank"
  :license {:name "MIT"
            :url "http://opensource.org/licenses/MIT"}
  :dependencies [[org.clojure/clojure "1.7.0"]
                 [instaparse "1.4.1"]]
  :main ^:skip-aot jank.core
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all}})
