(ns leiningen.jank.build
  (:require [babashka.process :as ps]
            [babashka.fs :as b.f]
            [clojure.pprint :as pp]
            [cemerick.pomegranate.aether :as aether])
  (:import (org.eclipse.aether.collection DependencyGraphTransformer)))

; TODO: Environment variable to override this.
(def parallel-job-count 2)
(def task-graph (atom {}))

(defn session []
  #(-> (aether/repository-session %)
       (.setDependencyGraphTransformer
         (reify DependencyGraphTransformer
           (transformGraph [_ root-node _context]
             root-node)))))

(defn enrich-graph [graph]
  (reduce (fn [acc [coord deps]]
            (let [artifact (first coord)
                  dep-artifacts (into #{} (map first deps))
                  acc (-> (update-in acc [artifact :dependencies] (fnil into #{}) dep-artifacts)
                          (assoc-in [artifact :status] :pending))]
              (reduce (fn [acc dep-artifact]
                        (update-in acc [dep-artifact :dependees] (fnil conj #{}) artifact))
                      acc
                      dep-artifacts)))
          {}
          graph))

; task, :blocked, or :done
(defn start-task!* [res task-graph]
  (let [pending (filter #(= :pending (:status (second %))) task-graph)]
    (if (empty? pending)
      (do
        (vreset! res :done)
        task-graph)
      (let [artifact (some (fn [[artifact props]]
                             (when (empty? (:dependencies props))
                               artifact))
                           pending)]
        (if (nil? artifact)
          (do
            (vreset! res :blocked)
            task-graph)
          (do
            (vreset! res artifact)
            (assoc-in task-graph [artifact :status] :started)))))))

(defn start-task! []
  (let [res (volatile! nil)]
    (swap! task-graph #(start-task!* res %))
    @res))

(defn finish-task!* [artifact task-graph]
  (let [task-graph (assoc-in task-graph [artifact :status] :done)]
    (reduce (fn [acc dep-artifact]
              (update-in acc [dep-artifact :dependencies] disj artifact))
            task-graph
            (get-in task-graph [artifact :dependees]))))

(defn finish-task! [artifact]
  (swap! task-graph #(finish-task!* artifact %)))

(defn build-deps! [project]
  (alter-var-root #'aether/maven-central assoc "clojars" "https://repo.clojars.org")
  ; TODO: Managed deps?
  (let [graph (-> (aether/resolve-dependencies :coordinates (:dependencies project)
                                               ;:coordinates '[[foo "0.1"]
                                               ;               [bar "0.1"]]
                                               :repository-session-fn (session))
                  enrich-graph)]
    (reset! task-graph graph)
    ;(pp/pprint @task-graph)
    (let [f #(loop []
               #_(when (= % "[thread 1]")
                 (pp/pprint @task-graph))
               (let [artifact (start-task!)]
                 (case artifact
                   :done nil
                   :blocked (do
                              ;(println (str % " Waiting to run more tasks..."))
                              (Thread/sleep 10)
                              (recur))
                   (do
                     (println (str % " Running " artifact))
                     ;(Thread/sleep 1000)
                     (finish-task! artifact)
                     (recur)))))
          threads (for [i (range parallel-job-count)]
                    (future (f (str "[thread " i "]"))))]
      (doseq [t threads]
        @t)
      nil)))
