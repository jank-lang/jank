(ns leiningen.jank.test-bb
  (:require [clojure.test :refer [deftest]]
            [babashka.process :as proc]))

;; Verify that jank-build loads without errors in Babashka.
(deftest load-jank-build-in-bb
  (->
   (proc/sh "bb -cp src/ -e \"(require 'jank-build.core)\"")
   (proc/check)))
