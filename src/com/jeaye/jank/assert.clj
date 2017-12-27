(ns com.jeaye.jank.assert
  (:require [com.jeaye.jank
             [log :refer [pprint]]]
            [com.jeaye.jank.parse
              [binding :as parse.binding]]))

(defn throw! []
  (assert false))

(defn form-source [source start-line end-line]
  (->> (clojure.string/split-lines source)
       (drop (dec start-line))
       (take (inc (- end-line start-line)))
       (clojure.string/join "\n")))

(defn underline [start-column end-column]
  (apply str (-> (vec (repeat start-column " "))
                 (into ["^"])
                 (into (repeat (- end-column start-column 2) "~")))))

(defn report! [prefix form-meta msg]
  (let [{:keys [:file
                :instaparse.gll/start-line
                :instaparse.gll/end-line
                :instaparse.gll/start-column
                :instaparse.gll/end-column]} form-meta]
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
    (report! "parse error" (meta form) msg)
    (throw!)))

(defn incomplete-parse [error]
  (report! "parse error"
           {:file parse.binding/*input-file*
            :instaparse.gll/start-line (:line error)
            :instaparse.gll/end-line (:line error)
            :instaparse.gll/start-column (-> error :column )
            :instaparse.gll/end-column (-> error :column )
            :instaparse.gll/start-index (:index error)
            :instaparse.gll/end-index (:index error)}
           (cond
             (= (:index error) (count parse.binding/*input-source*))
             "unexpected end of file"
             :else
             "invalid syntax"))
  (throw!))

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
