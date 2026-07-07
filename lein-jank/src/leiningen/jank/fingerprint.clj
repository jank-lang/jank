(ns leiningen.jank.fingerprint
  (:require [clojure.java.io :as io]
            [babashka.fs :as fs])
  (:import (java.security MessageDigest)
           (java.util Base64)))

(defn base64
  "Modified base64 encoding which is safe for URLs/file paths."
  [^bytes bs]
  (let [enc (-> (Base64/getUrlEncoder)
                .withoutPadding)]
    (.encodeToString enc bs)))

(defn fingerprint
  "Compute a fingerprint of some printable data structure."
  [data]
  ;; TODO: We should implement something like Cargo's fingerprint to better
  ;; react to changes in the environment:
  ;;
  ;; https://doc.rust-lang.org/nightly/nightly-rustc/cargo/core/compiler/fingerprint/index.html
  (let [md   (MessageDigest/getInstance "MD5")
        baos (java.io.ByteArrayOutputStream.)]
    (with-open [writer (io/writer baos)]
      (binding [*out* writer]
        (pr data)))
    (.update md (.toByteArray baos))
    (base64 (.digest md))))

(defn fingerprint-file
  "Like `fingerprint` but for the contents of a file."
  [f]
  (let [md (MessageDigest/getInstance "MD5")]
    (.update md (fs/read-all-bytes f))
    (base64 (.digest md))))
