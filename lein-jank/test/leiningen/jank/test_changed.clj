(ns leiningen.jank.test-changed
  (:require [clojure.test :refer [deftest is]]
            [babashka.fs :as fs]
            [leiningen.core.project :as proj]
            [leiningen.jank.changed :as changed]))

(def test-project (proj/read "test-project/project.clj"))

(deftest change-fingerprint
  (let [f1 (changed/change-fingerprint ["test-project/"] [])
        f2 (changed/change-fingerprint ["test-project/"] [])]
    (is (= f1 f2)))

  (let [f1 (changed/change-fingerprint ["test-project/"] [])
        _  (fs/touch "test-project/project.clj")
        f2 (changed/change-fingerprint ["test-project/"] [])]
    (is (not= f1 f2)))

  (is (= (with-redefs [changed/env (fn [ks] {"A" "B"})]
           (changed/change-fingerprint [] ["A"]))
         (with-redefs [changed/env (fn [ks] {"A" "B"})]
           (changed/change-fingerprint [] ["A"]))))

  (is (not= (with-redefs [changed/env (fn [ks] {"A" "B"})]
              (changed/change-fingerprint [] ["A"]))
            (with-redefs [changed/env (fn [ks] {"C" "D"})]
              (changed/change-fingerprint [] ["C"]))))

  (is (not= (with-redefs [changed/env (fn [ks] {"A" "B"})]
              (changed/change-fingerprint [] ["A"]))
            (with-redefs [changed/env (fn [ks] {"A" "NOT_B"})]
              (changed/change-fingerprint [] ["A"])))))
