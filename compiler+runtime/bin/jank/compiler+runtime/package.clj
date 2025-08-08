#!/usr/bin/env bb

(ns jank.compiler+runtime.package
  (:require [clojure.string]
            [jank.util :as util]
            [babashka.fs :as b.f]))

(def compiler+runtime-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../.."))))

(defn -main [{:keys [enabled?]}]
  (util/log-step "Create .deb package")
  (if (not enabled?)
    (util/log-info "Not enabled")
    (util/with-elapsed-time duration
      (let [; TODO: Non-hardcoded version.
            version "0.1"
            dir (format "jank_%s-1_amd64" version)
            control (format "Package: jank
Version: %s
Architecture: amd64
Maintainer: Jeaye Wilkerson <jeaye@jank-lang.org>
Depends: libssl-dev, gcc, libzip-dev, libxml2-dev, libstdc++-14-dev
Description: The native Clojure dialect hosted on LLVM with seamless C++ interop." version)]
        (util/quiet-shell {:dir compiler+runtime-dir}
                          (format "DESTDIR=%s ./bin/install" dir))
        (b.f/create-dir (str compiler+runtime-dir "/" dir "/DEBIAN"))
        (spit (str compiler+runtime-dir "/" dir "/DEBIAN/control") control)
        (util/quiet-shell {:dir compiler+runtime-dir}
                          (format "dpkg-deb --build --root-owner-group %s" dir))
        (when-some [gh-output (util/get-env "GITHUB_OUTPUT")]
          (spit gh-output (format "deb=%s.deb" dir))))
      (util/log-info-with-time duration "Created"))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true}))
