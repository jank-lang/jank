#!/usr/bin/env bb

(ns jank.compiler+runtime.package
  (:require [clojure.string :as str]
            [babashka.process :as b.p]
            [selmer.parser :as selmer]
            [jank.util :as util]
            [babashka.fs :as b.f]))

(def compiler+runtime-dir (str (b.f/canonicalize (str (b.f/parent *file*) "/../../.."))))
; TODO: Non-hardcoded version.
(def jank-version "0.1")

(defmulti create-package!
(fn [_props]
    (let [os (System/getProperty "os.name")]
      (cond
        (.startsWith os "Windows") :win
        (.startsWith os "Linux")   :linux
        (.startsWith os "Mac")     :mac
        :else                      :unknown))))

(defmethod create-package! :linux [_props]
  (let [group (or (System/getenv "JANK_PACKAGE_GROUP") "noble")
        libstdc++ (case group
                    "noble" "libstdc++-14-dev"
                    "libstdc++-16-dev")
        dir (format "jank_%s-%s-1_amd64" jank-version group)
        control (format "Package: jank
Version: %s-%s
Architecture: amd64
Section: devel
Priority: optional
Maintainer: Jeaye Wilkerson <jeaye@jank-lang.org>
Depends: libssl-dev, gcc, libbz2-dev, libzstd-dev, libxml2-dev, %s, zlib1g-dev, libboost-all-dev
Description: The native Clojure dialect with seamless C++ interop.
" jank-version group libstdc++)]
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

(defmethod create-package! :mac [_props]
  (let [dir (format "jank_%s-1_aarch64" jank-version)
        tarball (format "%s.tar.gz" dir)]
    (util/quiet-shell {:dir compiler+runtime-dir
                       :extra-env {"DESTDIR" dir}}
                      "./bin/install")
    (util/quiet-shell {:dir compiler+runtime-dir}
                      (format "tar czf %s %s" tarball dir))
    (when-some [gh-output (util/get-env "GITHUB_OUTPUT")]
      (b.f/copy (format "%s/%s" compiler+runtime-dir tarball) tarball)
      (spit gh-output (format "homebrew-tarball=%s" tarball)))))

(def win-depends
  "Runtime dependencies for the jank MSYS2 package."
  ["llvm-libs" "clang" "openssl"])

(def pkgbuild-template
  "Selmer template for generating the jank MSYS2 PKGBUILD.
   Assumes the source tree is already configured and compiled.
   Produces mingw-w64-clang-x86_64-jank-<pkgver>-<pkgrel>-any.pkg.tar.zst.
   Template params:
     pkgver  - package version
     pkgrel  - package release identifier
     deps    - list of runtime dep names without MINGW_PACKAGE_PREFIX"
  "_realname=jank
pkgbase=mingw-w64-${_realname}
pkgname=(\"${MINGW_PACKAGE_PREFIX}-${_realname}\")
pkgver={{pkgver}}
pkgrel={{pkgrel}}
pkgdesc=\"The native Clojure dialect on LLVM (mingw-w64)\"
arch=('any')
mingw_arch=('clang64')
url=\"https://jank-lang.org/\"
msys2_repository_url=\"https://github.com/jank-lang/jank\"
license=(\"spdx:MPL-2.0\")
depends=({% for dep in deps %}\"${MINGW_PACKAGE_PREFIX}-{{dep}}\"{% if not forloop.last %}
         {% endif %}{% endfor %})
makedepends=()
_pkgfn=.
source=()
sha256sums=()

build() {
  cd \"${srcdir}\"/${_pkgfn}/compiler+runtime
  ./bin/compile
}

package() {
  cd \"${srcdir}\"/${_pkgfn}/compiler+runtime
  DESTDIR=\"${pkgdir}\" cmake --install build
}
")

;; Packages jank into an MSYS2 pacman .pkg.tar.zst using makepkg.
;;
;; Requires: bin/configure + bin/compile already ran (build tree at
;; compiler+runtime-dir).
;;
;; Global inputs:
;;   compiler+runtime-dir - path to compiler+runtime/
;;   jank-version         - version string
;;   pkgbuild-template    - selmer PKGBUILD template to render for packaging
;;   win-depends          - runtime dependency list for the package
;;
;; Outputs: mingw-w64-clang-x86_64-jank-<pkgver>-<commit-epoch>.<commit>-any.pkg.tar.zst
;;   in build/makepkg-jank/.
;; When GITHUB_OUTPUT exists in the environment (i.e. on CI) writes
;;   path to it as msys2-pkg=<package-path>.
(defmethod create-package! :win [_props]
  (let [repo-root (b.f/canonicalize (b.f/path compiler+runtime-dir ".."))
        build-dir (b.f/path compiler+runtime-dir "build" "makepkg-jank")
        commit-epoch (str/trim (:out @(util/quiet-shell {:dir repo-root}
                                                        "git log -1 --format=%ct")))
        commit-hash (str/trim (:out @(util/quiet-shell {:dir repo-root}
                                                       "git rev-parse --short HEAD")))
        pkgbuild (selmer/render pkgbuild-template
                                {:pkgver jank-version
                                 :pkgrel (str commit-epoch "." commit-hash)
                                 :deps win-depends})
        pkgbuild-dest (b.f/path build-dir "PKGBUILD")]


    ;; DEBUG
    (util/log-info "compiler+runtime-dir: " compiler+runtime-dir)
    (util/log-info "repo-root: " repo-root)
    (util/log-info "portable-cmd: "
                   (util/command-make-portable "git log -1 --format=%ct"))
    (util/log-info "git-date: "
                   (pr-str  @(util/quiet-shell
                              {:dir repo-root}
                              "git log -1 --format=%ct")))

    ;; Clean and create the makepkg build directory
    (b.f/delete-tree build-dir)
    (b.f/create-dirs build-dir)

    (spit (str pkgbuild-dest) pkgbuild)

    ;; symlink repo into makpkg build as src
    (util/quiet-shell {:dir build-dir
                       :extra-env {"MSYS" "winsymlinks:nativestrict"}}
                      (format "ln -sf %s src" (b.f/unixify repo-root)))

    ;; build pkg
    (util/quiet-shell {:dir build-dir}
                      "makepkg --noextract -f --nodeps --noconfirm --nocheck")

    ;; report package
    (let [packages (b.f/glob build-dir "*.pkg.tar.zst")]
      (if (empty? packages)
        (do
          (util/log-error "Could not create package!")
          (System/exit 1))
        (let [pkg (first packages)]
          (util/log-info "Package: " pkg)
          (when-some [gh-output (util/get-env "GITHUB_OUTPUT")]
            (spit gh-output (format "msys2-pkg=%s" pkg))))))))

(defn -main [{:keys [enabled?] :as props}]
  (util/log-step "Create distro package")
  (if (not enabled?)
    (util/log-info "Not enabled")
    (util/with-elapsed-time duration
      (create-package! props)
      (util/log-info-with-time duration "Created"))))

(when (= *file* (System/getProperty "babashka.file"))
  (-main {:enabled? true}))
