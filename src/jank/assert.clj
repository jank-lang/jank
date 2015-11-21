(ns jank.assert)

(defn parse-assert [condition & msg]
  (assert condition (apply str "parse error: " msg)))

(defn type-assert [condition & msg]
  (assert condition (apply str "type error: " msg)))

(defn codegen-assert [condition & msg]
  (assert condition (apply str "codegen error: " msg)))
