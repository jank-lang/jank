(ns leiningen.jank.test-sandbox
  (:require [clojure.test :refer [deftest is testing]]
            [babashka.fs :as fs]
            [jank-build.sandbox.bwrap :as bwrap]
            [jank-build.sandbox.core :as sandbox]
            [jank-build.sandbox.seatbelt :as seatbelt]
            [jank-build.util :as util]))

(deftest sandbox-process-working-directory
  (let [seen (atom nil)]
    (with-redefs [util/sh (fn [opts cmd]
                  (reset! seen [opts cmd])
                  ::result)]
      (is (= ::result
   (sandbox/process false
                    [[:chdir "/tmp/lein-jank-sandbox-test"]]
                    {}
                    ["true"]))))
    (is (= "/tmp/lein-jank-sandbox-test" (get-in @seen [0 :dir])))))

(deftest bwrap-option-values
  (let [dir-path (str (fs/create-temp-dir {:prefix "lein-jank-bwrap-test"}))
        no-net   (bwrap/bwrap [[:tmpfs dir-path] [:chdir dir-path] [:net false]])
        with-net (bwrap/bwrap [[:tmpfs dir-path] [:net true]])]
    (is (some #{dir-path} no-net))
    (is (some #{"--chdir"} no-net))
    (is (not-any? #{"--share-net"} no-net))
    (is (some #{"--share-net"} with-net))))

(deftest seatbelt-profile
  (let [root    (fs/create-temp-dir {:prefix "lein-jank-seatbelt-test"})
        source  (fs/create-dirs (fs/path root "source"))
        output  (fs/create-dirs (fs/path root "output"))
        scratch (fs/create-dirs (fs/path root "scratch"))
        profile (seatbelt/profile [[:ro-bind source source]
                                   [:bind output output]
                                   [:tmpfs scratch]
                                   [:chdir scratch]
                                   [:net false]])]
    (testing "the profile is default deny and disables networking"
      (is (re-find #"\(deny default\)" profile))
      (is (re-find #"\(deny network\*\)" profile))
      (is (not (re-find #"\(allow network\*\)" profile))))
    (testing "declared paths are represented as escaped path filters"
      (is (re-find (re-pattern (str "\\(subpath \\\"" source "\\\"\\)"))
                   profile))
      (is (re-find (re-pattern (str "\\(subpath \\\"" output "\\\"\\)"))
                   profile))
      (is (re-find (re-pattern (str "\\(subpath \\\"" scratch "\\\"\\)"))
                   profile))
      (is (re-find
           (re-pattern (str "\\(allow process-exec \\(subpath \\\""
                            scratch
                            "\\\"\\)\\)"))
           profile)))))

(deftest seatbelt-rejects-remapped-bind
  (is (thrown-with-msg?
       IllegalArgumentException
       #"cannot remap"
       (seatbelt/profile [[:ro-bind "/source" "/different-destination"]]))))
