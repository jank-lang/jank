(ns jank.type.check)

(defn check [parsed]
  (loop [item (first (:cells parsed))
         remaining (rest (:cells parsed))
         checked []]
    (cond
      (empty? remaining)
      (conj checked item)
      :else
      (recur (first remaining)
             (rest remaining)
             (conj checked item)))))
