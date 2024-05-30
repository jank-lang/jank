# lein-jank

A Leiningen plugin to do many wonderful things with jank.

## Usage

Clone this repository. Execute `lein install` in the repository root.

Create a leiningen project: `lein new my-app`

Put `[lein-jank "0.0.1-SNAPSHOT"]` into the `:plugins` vector of your project.clj.

And voila, you can now run your jank files in the project.
`lein jank run <filepath>`

Make sure you have `jank` executable on your path.

This plugin delegates classpath generation to leiningen. Execute `lein classpath` to find your project's classpath.

Run the following to know more
```
$ lein jank help
```
