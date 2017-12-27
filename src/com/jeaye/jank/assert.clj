(ns com.jeaye.jank.assert
  (:require [com.jeaye.jank
             [log :refer [pprint]]]))

(defn report [prefix form msg]
  (pprint [form (meta form)])
  (let [file (:file (meta form))]
    (println (str file ": " prefix ": "))))

(defn parse-assert [condition form & msg]
  (when-not condition
    (report "parse error" form msg)))

;(defn type-assert [condition & msg]
;  (assert condition (apply str "type error: " msg)))
;
;(defn interpret-assert [condition & msg]
;  (assert condition (apply str "interpret error: " msg)))
;
;(defn codegen-assert [condition & msg]
;  (assert condition (apply str "codegen error: " msg)))
;
;(defn internal-assert [condition & msg]
;  (assert condition (apply str "internal error: " msg)))
;
;(defn not-yet-implemented [assert-fn condition & msg]
;  (assert-fn condition (apply str "not yet implemented: " msg)))
