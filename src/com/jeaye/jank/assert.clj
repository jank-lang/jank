(ns com.jeaye.jank.assert)

(defn parse-assert [condition & msg]
  (assert condition (apply str "parse error: " msg)))

(defn type-assert [condition & msg]
  (assert condition (apply str "type error: " msg)))

(defn interpret-assert [condition & msg]
  (assert condition (apply str "interpret error: " msg)))

(defn codegen-assert [condition & msg]
  (assert condition (apply str "codegen error: " msg)))

(defn internal-assert [condition & msg]
  (assert condition (apply str "internal error: " msg)))

(defn not-yet-implemented [assert-fn condition & msg]
  (assert-fn condition (apply str "not yet implemented: " msg)))
