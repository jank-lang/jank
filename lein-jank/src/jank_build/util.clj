(ns jank-build.util)

(defn warn [& args]
  (apply println args))

(defn abort [& args]
  (apply println args)
  (System/exit 1))
