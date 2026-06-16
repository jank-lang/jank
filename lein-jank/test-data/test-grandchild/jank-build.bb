(require '[test-build-dependency.core :as tbd])

(println "jank-build::define=I_LIKE=TURTLES")
(println "jank-build::include-dir=foobar")
(println "jank-build::link-dir=somepath")
(println "jank-build::link-library=foolib")
(println "jank-build::link-library=barlib")
(println "Hello jank build!")
(tbd/hello)
