(ns com.jeaye.jank.assert
  (:require [com.jeaye.jank
             [log :refer [pprint]]]
            [com.jeaye.jank.parse
              [binding :as parse.binding]]))

(defn form-source [source start-line end-line]
  (->> (clojure.string/split-lines source)
       (drop (dec start-line))
       (take (inc (- end-line start-line)))
       (clojure.string/join "\n")))

(defn underline [start-column end-column]
  (apply str (-> (into [] (repeat start-column " "))
                 (into ["^"])
                 (into (repeat (- end-column start-column 2) "~")))))

(defn report [prefix form msg]
  (let [{:keys [:file
                :instaparse.gll/start-line :instaparse.gll/end-line
                :instaparse.gll/start-column :instaparse.gll/end-column]} (meta form)]
    (println (str (apply str
                         file ":" start-line ":" start-column ": "
                         prefix ": "
                         msg)
                  "\n"
                  (form-source parse.binding/*input-source*
                               start-line end-line)
                  "\n"
                  (underline start-column end-column)))))

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
