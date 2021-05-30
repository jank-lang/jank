(ns com.jeaye.jank.optimize.spec
  (:require [clojure.spec.alpha :as s]
            [orchestra.core :refer [defn-spec]]
            [com.jeaye.jank.log :refer [pprint]]
            [com.jeaye.jank.parse.spec :as parse.spec]))

(s/def ::boxed? boolean?)
(s/def ::boxed-name ::parse.spec/name)
