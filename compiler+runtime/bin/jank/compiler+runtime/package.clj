#!/usr/bin/env bb

(ns jank.compiler+runtime.package
  (:require [clojure.string]
            [jank.util :as util]
            [babashka.fs :as b.f]))

(def compiler+runtime-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../.."))))
; TODO: Non-hardcoded version.
(def jank-version "0.1")

(defmulti create-package!
  (fn [_props]
    (System/getProperty "os.name")))

(defmethod create-package! "Linux" [_props]
  (let [dir (format "jank_%s-1_amd64" jank-version)
        control (format "Package: jank
                        Version: %s
                        Architecture: amd64
                        Maintainer: Jeaye Wilkerson <jeaye@jank-lang.org>
                        Depends: libssl-dev, gcc, libzip-dev, libxml2-dev, libstdc++-14-dev
                        Description: The native Clojure dialect hosted on LLVM with seamless C++ interop.
                        " jank-version)]
    (util/quiet-shell {:dir compiler+runtime-dir
                       :extra-env {"DESTDIR" dir}}
                      "./bin/install")
    (b.f/create-dir (str compiler+runtime-dir "/" dir "/DEBIAN"))
    (spit (str compiler+runtime-dir "/" dir "/DEBIAN/control") control)
    (util/quiet-shell {:dir compiler+runtime-dir}
                      (format "dpkg-deb --build --root-owner-group %s" dir))
    (when-some [gh-output (util/get-env "GITHUB_OUTPUT")]
      (b.f/copy (format "%s/%s.deb" compiler+runtime-dir dir) (format "%s.deb" dir))
      (spit gh-output (format "deb=%s.deb" dir)))))

(defmethod create-package! "Mac OS X" [_props]
  (let [dir (format "jank_%s-1_aarch64" jank-version)
        tarball (format "%s.tar.gz" dir)]
    (util/quiet-shell {:dir compiler+runtime-dir
                       :extra-env {"DESTDIR" dir}}
                      "./bin/install")
    (util/quiet-shell {:dir compiler+runtime-dir}
                      (format "tar xf %s %s" tarball dir))
    (when-some [gh-output (util/get-env "GITHUB_OUTPUT")]
      (b.f/copy (format "%s/%s" compiler+runtime-dir tarball) tarball)
      (spit gh-output (format "homebrew-tarball=%s" tarball)))))

(defn -main [{:keys [enabled?] :as props}]
  (util/log-step "Create distro package")
  (if (not enabled?)
    (util/log-info "Not enabled")
    (util/with-elapsed-time duration
      (create-package! props)
      (util/log-info-with-time duration "Created"))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true}))
