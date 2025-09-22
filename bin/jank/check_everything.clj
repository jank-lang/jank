#!/usr/bin/env bb

(ns jank.check-everything
  (:require [babashka.fs :as b.f]
            [jank.compiler+runtime.core]
            [jank.clojure-cli.core]
            [jank.lein-jank.core]
            [jank.summary :as summary]
            [jank.util :as util]))

(defn show-env []
  (util/log-info "JANK_MATRIX_ID: " (System/getenv "JANK_MATRIX_ID"))
  (util/log-info "JANK_INSTALL_DEPS: " (System/getenv "JANK_INSTALL_DEPS"))
  (util/log-info "JANK_BUILD_TYPE: " (System/getenv "JANK_BUILD_TYPE"))
  (util/log-info "JANK_LINT: " (System/getenv "JANK_LINT"))
  (util/log-info "JANK_COVERAGE: " (System/getenv "JANK_COVERAGE"))
  (util/log-info "JANK_ANALYZE: " (System/getenv "JANK_ANALYZE"))
  (util/log-info "JANK_SANITIZE: " (System/getenv "JANK_SANITIZE"))
  (util/log-info "JANK_PACKAGE: " (System/getenv "JANK_PACKAGE")))

(defn install-common-deps []
  ; TODO: Enable once we're linting Clojure/jank again.
  ;(util/quiet-shell {} "sudo npm install --global @chrisoakman/standard-clojure-style")

  ; TODO: Cache this shit.
  (when (= "on" (util/get-env "JANK_ANALYZE"))
    (util/quiet-shell {} "curl -Lo clang-tidy-cache https://raw.githubusercontent.com/matus-chochlik/ctcache/refs/heads/main/src/ctcache/clang_tidy_cache.py")
    (util/quiet-shell {} "chmod +x clang-tidy-cache")
    (util/quiet-shell {} "sudo mv clang-tidy-cache /usr/local/bin")
    (let [clang-tidy (util/find-llvm-tool "clang-tidy")]
      (spit "clang-tidy-cache-wrapper"
            (str "#!/bin/bash\nclang-tidy-cache " clang-tidy " \"${@}\"")))
    (util/quiet-shell {} "chmod +x clang-tidy-cache-wrapper")
    (util/quiet-shell {} "sudo mv clang-tidy-cache-wrapper /usr/local/bin")))

(defmulti install-deps
  (fn [_props]
    (System/getProperty "os.name")))

(defmethod install-deps "Linux" [{:keys [validate-formatting?]}]
  (install-common-deps)
  ; TODO: Enable once we're not building Clang/LLVM from source again.
  ;; Install Clang/LLVM.
  ;(util/quiet-shell {} "curl -L -O https://apt.llvm.org/llvm.sh")
  ;(util/quiet-shell {} "chmod +x llvm.sh")
  ;(util/quiet-shell {} (str "sudo ./llvm.sh " util/llvm-version " all"))
  ;; The libc++abi headers conflict with the system headers:
  ;; https://github.com/llvm/llvm-project/issues/121300
  ;(util/quiet-shell {} (str "sudo apt-get remove -y libc++abi-" util/llvm-version "-dev"))

  ; Install the new Clojure CLI.
  (util/quiet-shell {} "curl -L -O https://github.com/clojure/brew-install/releases/latest/download/linux-install.sh")
  (util/quiet-shell {} "chmod +x linux-install.sh")
  (util/quiet-shell {} "sudo ./linux-install.sh"))

(defmethod install-deps "Mac OS X" [_props]
  (install-common-deps))

(defn -main [{:keys [install-deps? validate-formatting? compiler+runtime
                     clojure-cli lein-jank]
              :as props}]
  (summary/initialize)

  (util/log-boundary "Show environment")
  (show-env)

  (util/log-boundary "Install dependencies")
  (if-not install-deps?
    (util/log-info "Not enabled")
    (util/with-elapsed-time duration
      (install-deps props)
      (util/log-info-with-time duration "Dependencies installed")))

  (jank.compiler+runtime.core/-main {:validate-formatting? validate-formatting?
                                     :build? (:build? compiler+runtime)})

  #_(jank.clojure-cli.core/-main {:validate-formatting? validate-formatting?
                                :build? (:build? clojure-cli)})

  #_(jank.lein-jank.core/-main {:validate-formatting? validate-formatting?
                              :build? (:build? lein-jank)}))

(when (= *file* (System/getProperty "babashka.file"))
  (let [build? (some? (util/get-env "JANK_BUILD_TYPE"))]
    (-main {:install-deps? (parse-boolean (util/get-env "JANK_INSTALL_DEPS" "false"))
            :validate-formatting? (parse-boolean (util/get-env "JANK_LINT" "false"))
            :compiler+runtime {:build? build?}
            :clojure-cli {:build? build?}
            :lein-jank {:build? build?}})))
