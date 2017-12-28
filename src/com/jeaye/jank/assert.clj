(ns com.jeaye.jank.assert
  (:require [com.jeaye.jank
             [log :refer [pprint]]]
            [com.jeaye.jank.parse
              [binding :as parse.binding]]))

(defn throw! []
  (assert false))

(defn form-source
  "Returns the snippet of source within the start and end lines."
  [source start-line end-line]
  (->> (clojure.string/split-lines source)
       (drop (dec start-line))
       (take (inc (- end-line start-line)))
       (clojure.string/join "\n")))

(defn whitespace? [ch]
  (or (= \space ch) (= \newline ch)))

(defn form-start-column
  "Returns an adjusted start column for a form. Instaparse groups leading
   whitespace with the matched token, but that's not useful in error reporting.
   This detects whitespace and adjusts the start column to where the token
   actually starts."
  [form-meta]
  (let [{:keys [:instaparse.gll/start-column
                :instaparse.gll/start-index]} form-meta]
    (if (whitespace? (nth parse.binding/*input-source* start-index))
      (let [; Get the chunk of leading whitespace.
            whitespace (->> (subs parse.binding/*input-source* start-index)
                            (re-seq #"\s+")
                            first)
            ; If there is a new line, then the token start is however many
            ; spaces there are after the last whitespace. Otherwise, it's
            ; the old start, plus the leading whitespace.
            multiple-lines? (clojure.string/includes? whitespace "\n")]
        (if multiple-lines?
          (->> (re-seq #" +" whitespace) last count inc)
          (+ start-column (count whitespace))))
      start-column)))

(defn underline
  "Builds an underline string lead by whitespace, spanning the specified columns.
   Columns are both inclusive."
  [start-column end-column]
  (apply str (-> (vec (repeat (dec start-column) " "))
                 (into ["^"])
                 (into (repeat (- end-column start-column) "~")))))

(defn report!
  "Prints an error report containing the relevant file/line/column info, a code
   snippet of the relevant form, and a helpful underline."
  [prefix form-meta msg]
  (let [{:keys [:file
                :instaparse.gll/start-line
                :instaparse.gll/end-line]} form-meta
        start-column (form-start-column form-meta)
        ; Instaparse uses an exclusive end column, but we want inclusive.
        end-column (-> form-meta :instaparse.gll/end-column dec)
        sections [(apply str
                         file ":" start-line ":" start-column ": "
                         prefix ": "
                         msg)
                  (form-source parse.binding/*input-source*
                               start-line end-line)
                  (underline start-column end-column)]]
    (println (clojure.string/join "\n" sections))))

(defn parse-assert! [condition form & msg]
  (when-not condition
    (report! "parse error" (meta form) msg)
    (throw!)))

(defn incomplete-parse!
  "Triggers and assertion failuire for a parse which could not be finished. In
   this specific case, there isn't a parse tree with meta data for each form,
   so the meta data is built from the parse error instead."
  [error]
  (let [; The index may be out of bounds, in the case where there was an
        ; unmatched delimeter.
        end? (= (:index error) (count parse.binding/*input-source*))
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
