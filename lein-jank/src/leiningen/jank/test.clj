(ns leiningen.jank.test
  (:require [leiningen.jank.discovery :as d]
            [clojure.java.io :as io]
            [clojure.string :as string]
            [leiningen.core.main :as lmain]
            [leiningen.core.project :as p]
            [leiningen.jank.core :as ljc]
            [babashka.fs :as fs]))

(defn- convert-to-ns [possible-file]
  (if (d/valid-source-file? (io/file possible-file))
    (d/file->jank-ns possible-file)
    possible-file))

(defn- split-selectors [args]
  (let [[nses selectors] (split-with (complement keyword?) args)]
    [nses
     (loop [acc {} [selector & selectors] selectors]
       (if (seq selectors)
         (let [[args next] (split-with (complement keyword?) selectors)]
           (recur (assoc acc selector (list 'quote args))
                  next))
         (if selector
           (assoc acc selector ())
           acc)))]))

(defn- partial-selectors [project-selectors selectors]
  (for [[k v] selectors
        :let [selector-form (k project-selectors)]
        :when selector-form]
    [selector-form v]))

(def ^:private only-form
  ['(fn [ns & vars]
      ((set (for [v vars]
              (-> (str v)
                  (clojure.string/split #"/")
                  (first)
                  (symbol))))
       ns))
   '(fn [m & vars]
      (some #(let [var (str "#'" %)]
               (if (some #{\/} var)
                 (= var (-> m ::var str))
                 (= % (ns-name (:ns m)))))
            vars))])

(defn ^:internal read-args [args project]
  (let [args (->> args (map convert-to-ns) (map read-string))
        [nses given-selectors] (split-selectors args)
        nses (or (seq nses)
                 (sort (d/jank-namespaces (distinct (:test-paths project)))))
        selectors (partial-selectors (merge {:all '(constantly true)}
                                            {:only only-form}
                                            (:test-selectors project))
                                     given-selectors)
        selectors-or-default (if (and (empty? selectors)
                                      (:default (:test-selectors project)))
                               [[(:default (:test-selectors project)) ()]]
                               selectors)]
    (when (and (empty? selectors)
               (seq given-selectors))
      (lmain/abort "Please specify :test-selectors in project.clj"))
    [nses selectors-or-default]))

(def form-for-suppressing-unselected-tests
  "A function that figures out which vars need to be suppressed based on the
  given selectors, moves their :test metadata to :leiningen/skipped-test (so
  that clojure.test won't think they are tests), runs the given function, and
  then sets the metadata back."
  `(fn [namespaces# selectors# func#]
     (let [copy-meta# (fn [var# from-key# to-key#]
                        (if-let [x# (get (meta var#) from-key#)]
                          (alter-meta! var# #(-> % (assoc to-key# x#) (dissoc from-key#)))))
           vars# (if (seq selectors#)
                   (->> namespaces#
                        (mapcat (comp vals ns-interns))
                        (remove (fn [var#]
                                  (some (fn [[selector# args#]]
                                          (let [sfn# (if (vector? selector#)
                                                       (second selector#)
                                                       selector#)]
                                            (apply sfn#
                                                   (merge (-> var# meta :ns meta)
                                                          (assoc (meta var#) ::var var#))
                                                   args#)))
                                        selectors#)))))
           copy# #(doseq [v# vars#] (copy-meta# v# %1 %2))]
       (copy# :test :leiningen/skipped-test)
       (try (func#)
            (finally
              (copy# :leiningen/skipped-test :test))))))

(defn- form-for-select-namespaces [namespaces selectors]
  `(reduce (fn [acc# [f# args#]]
             (if (vector? f#)
               (filter #(apply (first f#) % args#) acc#)
               acc#))
           '~namespaces ~selectors))

(defn- form-for-nses-selectors-match [selectors ns-sym]
  `(distinct
    (for [ns# ~ns-sym
          [_# var#] (ns-publics ns#)
          :when (and (clojure.test/-workaround-get-test var#)
                     (some (fn [[selector# args#]]

                             (apply (if (vector? selector#)
                                      (second selector#)
                                      selector#)
                                    (merge (-> var# meta :ns meta)
                                           (assoc (meta var#) ::var var#))
                                    args#))
                           ~selectors))]
      ns#)))

(defn- form-for-testing-namespaces [nses selectors]
  (let [ns-sym (gensym "namespaces")]
    `(let [~ns-sym ~(form-for-select-namespaces nses selectors)]
       (when (seq ~ns-sym)
         (apply require ~ns-sym))
       (let [selected-namespaces# ~(form-for-nses-selectors-match selectors ns-sym)
             summary# (binding [clojure.test/*test-out* *out*]
                        (~form-for-suppressing-unselected-tests
                         selected-namespaces# ~selectors
                         #(apply ~'clojure.test/run-tests selected-namespaces#)))]
         (let [exit-code# (min 1
                               (+ (int (:error summary#))
                                  (int (:fail summary#))))]
           (cpp/exit exit-code#))))))

(defn- generate-test-runner! [form]
  (let [test-runner-file (fs/create-temp-file {:prefix "jank_test_runner"
                                               :suffix ".jank"})]
    (spit (fs/file test-runner-file)
          (string/join
           "\n"
           [(pr-str `(require 'clojure.string))
            (pr-str `(require 'clojure.test))
            (pr-str form)]))

    test-runner-file))

(defn test! [project & args]
  (let [project (p/merge-profiles project [:leiningen/test :test])
        cp-str (ljc/build-module-path project)
        [nses selectors] (read-args args project)
        form (form-for-testing-namespaces nses (vec selectors))

        test-runner-file (generate-test-runner! form)]
    (ljc/shell-out! project cp-str "run" [(str test-runner-file)] [])))
