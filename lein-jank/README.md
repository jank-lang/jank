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

## License

Copyright © 2024 FIXME

This program and the accompanying materials are made available under the
terms of the Eclipse Public License 2.0 which is available at
http://www.eclipse.org/legal/epl-2.0.

This Source Code may also be made available under the following Secondary
Licenses when the conditions for such availability set forth in the Eclipse
Public License, v. 2.0 are satisfied: GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or (at your
option) any later version, with the GNU Classpath Exception which is available
at https://www.gnu.org/software/classpath/license.html.
