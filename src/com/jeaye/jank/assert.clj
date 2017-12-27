(ns com.jeaye.jank.assert
  (:require [com.jeaye.jank
             [log :refer [pprint]]]
            [com.jeaye.jank.parse
              [binding :as parse.binding]]))

(defn report [prefix form msg]
  (pprint [form (meta form)])
  (let [{:keys [:file
                :instaparse.gll/start-line :instaparse.gll/start-column
                :instaparse.gll/start-index :instaparse.gll/end-index]} (meta form)]
    (println (apply str
                    file ":" start-line ":" start-column ": "
                    prefix ": "
                    msg "\n"
                    (pr-str form)))))

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
