(ns jank.parse
  (:require [instaparse.core :as insta]))

(def parse
  (insta/parser
    (clojure.java.io/resource "grammar")
    :auto-whitespace :standard))
