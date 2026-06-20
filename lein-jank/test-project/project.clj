(defproject test-project "0.1.0-SNAPSHOT"
  :description "FIXME: write description"
  :url "http://example.com/FIXME"
  :plugins [[org.jank-lang/lein-jank "0.7"]]
  ;; include a native dependency for testing
  :dependencies [[org.clojars.kylc/jank-glfw3 "0.1-SNAPSHOT"]]
  :main test-project.core
  :jank {;:disable-locals-clearing false
         ;:elide-meta false
         ;:direct-call false
         :optimization-level 2
         ;:codegen :llvm-ir
         ;; -D
         ;:defines {}
         ;; -I
         ;:include-dirs []
         ;; -L
         ;:library-dirs []
         ;; -l
         ;:linked-libraries []
         }
  :profiles {:release {:jank {}}
             :test {:test-paths ["tests"]}})
