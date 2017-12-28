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

(defn whitespace? [ch]
  (or (= \space ch) (= \newline ch)))

(defn form-start-column [form-meta]
  (let [{:keys [:instaparse.gll/start-column
                :instaparse.gll/start-index]} form-meta]
    (if (whitespace? (nth parse.binding/*input-source* start-index))
      (let [whitespace (->> (subs parse.binding/*input-source* start-index)
                            (re-seq #"\s+")
                            first)
            multiple-lines? (clojure.string/includes? whitespace "\n")]
        (if multiple-lines?
          (->> (re-seq #" +" whitespace) last count inc)
          (+ start-column (count whitespace))))
      start-column)))

(defn underline [start-column end-column]
  (apply str (-> (vec (repeat (dec start-column) " "))
                 (into ["^"])
                 (into (repeat (- end-column start-column) "~")))))

(defn report! [prefix form-meta msg]
  (let [{:keys [:file
                :instaparse.gll/start-line
                :instaparse.gll/end-line]} form-meta
        start-column (form-start-column form-meta)
        end-column (-> form-meta :instaparse.gll/end-column dec)]
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
  (let [end? (= (:index error) (count parse.binding/*input-source*))
        msg (cond
              end?
              "unexpected end of file"
              :else
              "invalid syntax")
        index (if end? ; TODO: Determine where it started
                (-> error :index dec)
                (:index error))]
    (report! "parse error"
             {:file parse.binding/*input-file*
              :instaparse.gll/start-line (:line error)
              :instaparse.gll/end-line (:line error)
              :instaparse.gll/start-column (:column error)
              :instaparse.gll/end-column (:column error)
              :instaparse.gll/start-index index
              :instaparse.gll/end-index index}
             msg)
    (throw!)))

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
