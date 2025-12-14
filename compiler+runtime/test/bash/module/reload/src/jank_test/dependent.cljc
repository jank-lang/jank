(ns jank-test.dependent
  (:require [jank-test.dependency :as dep]))

(defn first-fn []
  (dep/say-hello!))
