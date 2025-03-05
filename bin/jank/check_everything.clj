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
  (util/log-info "JANK_SANITIZE: " (System/getenv "JANK_SANITIZE")))

; Most Linux deps are installed by a Github action. We need to manually install
; boost for some reason. Otherwise, its headers aren't found by clang.
(def os->deps-cmd {"Mac OS X" "brew install curl git git-lfs zip entr openssl double-conversion pkg-config ninja python cmake gnupg zlib doctest boost libzip lbzip2 llvm@19"})

(defmulti install-deps
  (fn [_props]
    (System/getProperty "os.name")))

(defmethod install-deps "Linux" [{:keys [validate-formatting?]}]
  ; TODO: Cache this shit.
  (when (= "on" (util/get-env "JANK_ANALYZE"))
    (util/quiet-shell {} "curl -Lo clang-tidy-cache https://raw.githubusercontent.com/matus-chochlik/ctcache/refs/heads/main/src/ctcache/clang_tidy_cache.py")
    (util/quiet-shell {} "chmod +x clang-tidy-cache")
    (util/quiet-shell {} "sudo mv clang-tidy-cache /usr/local/bin")
    (spit "clang-tidy-cache-wrapper"
          "#!/bin/bash
           clang-tidy-cache clang-tidy \"${@}\"")
    (util/quiet-shell {} "chmod +x clang-tidy-cache-wrapper")
    (util/quiet-shell {} "sudo mv clang-tidy-cache-wrapper /usr/local/bin"))

  ; TODO: Enable once we're linting Clojure/jank again.
  ;(util/quiet-shell {} "sudo npm install --global @chrisoakman/standard-clojure-style")

  ; Install Clang/LLVM.
  (util/quiet-shell {} "curl -L -O https://apt.llvm.org/llvm.sh")
  (util/quiet-shell {} "chmod +x llvm.sh")
  (util/quiet-shell {} (str "sudo ./llvm.sh " util/llvm-version " all"))
  ; The libc++abi headers conflict with the system headers:
  ; https://github.com/llvm/llvm-project/issues/121300
  (util/quiet-shell {} (str "sudo apt-get remove -y libc++abi-" util/llvm-version "-dev"))

  ; Install the new Clojure CLI.
  (util/quiet-shell {} "curl -L -O https://github.com/clojure/brew-install/releases/latest/download/linux-install.sh")
  (util/quiet-shell {} "chmod +x linux-install.sh")
  (util/quiet-shell {} "sudo ./linux-install.sh"))

(defmethod install-deps "Mac OS X" [_props]
  (util/quiet-shell {:extra-env {"HOMEBREW_NO_AUTO_UPDATE" "1"}}
                    (os->deps-cmd "Mac OS X"))

  ; TODO: This is missing some of the other things above.
  )

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

  (jank.clojure-cli.core/-main {:validate-formatting? validate-formatting?
                                :build? (:build? clojure-cli)})

  (jank.lein-jank.core/-main {:validate-formatting? validate-formatting?
                              :build? (:build? lein-jank)}))

(when (= *file* (System/getProperty "babashka.file"))
  (let [build? (some? (util/get-env "JANK_BUILD_TYPE"))]
    (-main {:install-deps? (parse-boolean (util/get-env "JANK_INSTALL_DEPS" "true"))
            :validate-formatting? (parse-boolean (util/get-env "JANK_LINT" "false"))
            :compiler+runtime {:build? build?}
            :clojure-cli {:build? build?}
            :lein-jank {:build? build?}})))
