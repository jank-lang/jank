(defproject org.jank-lang/test-managed-deps "0.1.0"
  :description "FIXME: write description"
  :url "https://example.com/FIXME"
  :license {:name "MPL 2.0"
            :url  "https://www.mozilla.org/en-US/MPL/2.0/"}
  :plugins [[org.jank-lang/lein-jank "LATEST"]]
  :middleware [leiningen.jank/middleware]
  :managed-dependencies [[org.jank-lang.commons/imgui-sys "2026.06-1"]]
  :dependencies [[org.jank-lang.commons/imgui-sys nil]
                 [org.jank-lang.commons/imgui-opengl2-sys "2026.06-6"]
                 [org.jank-lang.commons/imgui-glfw-sys "2026.06-1"]])
