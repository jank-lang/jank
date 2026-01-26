(ns jank-test.reload-all.dependent
  (:require [jank-test.reload-all.dependency :as dep]))

(defn first-fn []
  (dep/say-hello!))
