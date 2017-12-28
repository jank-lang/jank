(ns com.jeaye.jank.assert.report
  (:require [io.aviso.ansi :as ansi]
            ; TODO: Implement custom highlighter. There's no need to parse the
            ; source again when we already have the parse tree.
            [glow.core :as glow]
            [com.jeaye.jank
             [log :refer [pprint]]]
            [com.jeaye.jank.parse
             [binding :as parse.binding]]))

(def file-color ansi/green-font)
(def error-color ansi/red-font)
(def syntax-colors {:exception :green
                    :repeat  :green
                    :conditional :green
                    :variable :blue
                    :core-fn :blue
                    :definition :bright-red
                    :reader-char :yellow
                    :special-form :bright-red
                    :macro :bright-red
                    :number :cyan
                    :boolean :cyan
                    :nil :cyan
                    :s-exp :default
                    :keyword :green
                    :comment :bright-green
                    :string :cyan
                    :character :cyan
                    :regex :yellow})

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

(defn dump!
  "Prints an error report containing the relevant file/line/column info, a code
   snippet of the relevant form, and a helpful underline."
  [prefix form-meta msg highlight?]
  (let [{:keys [:file
                :instaparse.gll/start-line
                :instaparse.gll/end-line]} form-meta
        start-column (form-start-column form-meta)
        ; Instaparse uses an exclusive end column, but we want inclusive.
        end-column (-> form-meta :instaparse.gll/end-column dec)
        ; We don't highlight parse errors, since the code isn't syntactically
        ; correct and isn't worth trying to highlight.
        highlight (if highlight?
                    #(glow/highlight % syntax-colors)
                    identity)
        sections [(apply str
                         file-color file ":" start-line ":" start-column ": "
                         error-color prefix ": "
                         ansi/reset-font msg)
                  (highlight (form-source parse.binding/*input-source*
                                          start-line end-line))
                  (str error-color
                       (underline start-column end-column)
                       ansi/reset-font)]]
    (println (clojure.string/join "\n" sections))))
