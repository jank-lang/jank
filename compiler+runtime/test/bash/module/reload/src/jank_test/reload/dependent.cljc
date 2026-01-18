(ns jank-test.reload.dependent
  (:require [jank-test.reload.dependency :as dep]))

(defn first-fn []
  (dep/say-hello!))
