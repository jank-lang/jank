# lein-jank

A Leiningen plugin to do many wonderful things with jank.

## Usage

- Execute `lein install` in the `lein-jank`'s root..
- Create a leiningen project: `lein new my-app`
- Update the name of the source file `src/my_app/core.clj` to be a jank file i.e. `src/my_app/core.jank`
- Add a `-main` function to the `core.jank`
```clojure
(defn -main [& args]
  (println "Hello, world!")
  (println "Args: " args))
```
- Put `[org.jank-lang/lein-jank "0.0.1-SNAPSHOT"]` into the `:plugins` vector of your project.clj.
- Set the `:main` key in the project.clj to `my-app.core`
```clojure
(defproject my-app "v0.0.1"
  ...
  :plugins [[org.jank-lang/lein-jank "0.0.1-SNAPSHOT"]]
  :main my-app.core
  ...)
```
- Run the project: `lein jank run <args>`

Make sure you have `jank` executable on your path.

This plugin delegates classpath generation to leiningen. Execute `lein classpath` to find your project's classpath.

Run the following to know more
```
$ lein jank help
```
