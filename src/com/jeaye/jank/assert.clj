(ns com.jeaye.jank.assert
  (:require [com.jeaye.jank
             [log :refer [pprint]]]
            [com.jeaye.jank.parse
              [binding :as parse.binding]]
            [com.jeaye.jank.assert.report :as report]))

(defn throw! [dump]
  (throw (ex-info (:prefix dump) {:dump dump})))

(defn parse-assert! [condition form & msg]
  (when-not condition
    (throw! (report/dump! "parse error" (meta form) msg false))))

(defn type-assert! [condition form & msg]
  (when-not condition
    (throw! (report/dump! "type error" (meta form) msg true))))

(defn incomplete-parse!
  "Triggers and assertion failure for a parse which could not be finished. In
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
                (:index error))
        dump (report/dump! "parse error"
                           {:file parse.binding/*input-file*
                            :instaparse.gll/start-line (:line error)
                            :instaparse.gll/end-line (:line error)
                            :instaparse.gll/start-column (:column error)
                            :instaparse.gll/end-column (:column error)
                            :instaparse.gll/start-index index
                            :instaparse.gll/end-index index}
                           msg
                           false)]
    (throw! dump)))

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
