#!/usr/bin/env bb

(ns jank.check-everything
  (:require [jank.compiler+runtime.core]
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

(def os->deps-cmd {"Linux" "sudo apt-get install -y curl git git-lfs zip build-essential entr libssl-dev libdouble-conversion-dev pkg-config ninja-build cmake zlib1g-dev libffi-dev libzip-dev libbz2-dev doctest-dev libboost-all-dev gcc g++ libgc-dev"

                   "Mac OS X" "brew install curl git git-lfs zip entr openssl double-conversion pkg-config ninja python cmake gnupg zlib doctest boost libzip lbzip2 llvm@19"})

; TODO: Cache these deps using https://github.com/actions/cache/
; Maybe follow this sort of thing: https://github.com/gerlero/apt-install/blob/main/action.yml
(defmulti install-deps
  (fn []
    (System/getProperty "os.name")))

(defmethod install-deps "Linux" []
  (let [apt? (try
               (util/quiet-shell {} "which apt-get")
               true
               (catch Exception _e
                 false))]
    (if-not apt?
      (util/log-warning "Skipping dependency install, since we don't have apt-get")
      (do
        (util/quiet-shell {} "sudo apt-get update -y")
        ; Install deps required for running our tests.
        (util/quiet-shell {} "sudo apt-get install -y default-jdk software-properties-common lsb-release npm lcov leiningen")
        ; TODO: Enable once we're linting Clojure/jank again.
        ;(util/quiet-shell {} "sudo npm install --global @chrisoakman/standard-clojure-style")

        ; Install jank's build deps.
        (util/quiet-shell {} (os->deps-cmd "Linux"))

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
        (util/quiet-shell {} "sudo ./linux-install.sh")))))

(defmethod install-deps "Mac OS X" []
  (util/quiet-shell {:extra-env {"HOMEBREW_NO_AUTO_UPDATE" "1"}}
                    (os->deps-cmd "Mac OS X")))

(defn -main [{:keys [install-deps? validate-formatting? compiler+runtime
                     clojure-cli lein-jank]}]
  (summary/initialize)

  (util/log-boundary "Show environment")
  (show-env)

  (util/log-boundary "Install dependencies")
  (if-not install-deps?
    (util/log-info "Not enabled")
    (util/with-elapsed-time duration
      (install-deps)
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
