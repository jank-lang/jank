## Testing a Leiningen project
Leiningen has support for easily running all tests for a project. Tests are
written in the `test/` directory. The jank template provided us with an example
test which will fail.

```bash
$ lein test
```

If tests exist in directories other than `test/`, you can use the `:test`
profile to point Leiningen to them. Leiningen will find all jank source files
within those directories, recursively, and ensure their tests run.

```clojure
(defproject hello_lein "0.1-SNAPSHOT"
  ;; ...
  :profiles {;; ...
             :test {:test-paths ["src/test" "other/test"]}})

```

### Running specific tests
You can run tests in specific namespaces by specifying them as command-line
arguments.

```bash
$ lein test project.ns1 project.ns2
```

You can also run just specific tests.

```bash
$ lein test :only project.ns1/my-test
```

#### Test selectors
Test selectors allow you to define filters for which tests to run.
When you execute `lein test`, Leiningen uses the `:default` selector 
to select the tests, by default. However, you can add new selectors in
`project.clj` and specify them when running `lein test`.

For example, let's say our integration tests are slow and we want to separate
them from our other tests. First we add a new `:integration` test selector to
our project.

```clojure
(defproject hello_lein "0.1-SNAPSHOT"
  ;; ...
  :profiles {;; ...
             :test {:test-selectors {:default (complement :integration)
                                     :integration :integration}}})
```

Then we tag our integration tests with the `:integration` metadata.

```clojure
(deftest ^:integration network-heavy-test
  (is (= [1 2 3] (:numbers (network-operation)))))
```

Finally, we can run only the integration tests.

```bash
$ lein test :integration
```
