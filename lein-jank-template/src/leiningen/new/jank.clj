(ns leiningen.new.jank
  (:require [leiningen.new.templates :refer [renderer project-name
                                             ->files sanitize-ns name-to-path
                                             multi-segment]]
            [leiningen.core.main :as main]))

(defn jank [name]
  (let [render (renderer "jank")
        main-ns (multi-segment (sanitize-ns name))
        data {:raw-name name
              :name (project-name name)
              :namespace main-ns
              :nested-dirs (name-to-path main-ns)}]
    (main/info "Generating a project called" name "based on the 'jank' template.")
    (->files data
             ["project.clj" (render "project.clj" data)]
             ["src/{{nested-dirs}}.jank" (render "main.jank" data)]
             ["test/{{nested-dirs}}_test.jank" (render "test.jank" data)]
             ["LICENSE" (render "LICENSE" data)])))
