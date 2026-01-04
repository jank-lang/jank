(ns jank-test.a)

(defmacro with-prelog [& body]
  (println :only-during-read)
  `(do ~@body))

(with-prelog
  (defn -main []
    (println :success)))
