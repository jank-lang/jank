(defproject {{raw-name}} "0.1.0-SNAPSHOT"
  :dependencies []
  :plugins [[org.jank-lang/lein-jank "0.1.0-SNAPSHOT"]]
  :jank {:main {{namespace}}}
  :profiles {:debug {:jank {:optimization-level 0}}
             :release {:jank {:optimization-level 2}}})
