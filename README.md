# The jank programming language [![Build Status](https://travis-ci.org/jeaye/jank.svg?branch=master)](https://travis-ci.org/jeaye/jank) [![codecov](https://codecov.io/gh/jeaye/jank/branch/master/graph/badge.svg)](https://codecov.io/gh/jeaye/jank)

jank is a Clojure dialect which aims to offer the best of both worlds: dynamic
and static. For both of these worlds, jank uses a single runtime, so the
transition from one to the other is not only seamless, it can be gradual.

## Runtime
| Dynamic Runtime      | Static Runtime                                                     |
|----------------------|--------------------------------------------------------------------|
| Eval code with a JIT | AOT compile to native executable                                   |
| REPL support         | Whole program analysis                                             |
| Code-as-data macros  | Seamless C and C++ interop (including templates, inheritance, etc) |

## Typing
| Dynamic Typing                              | Static Typing                                                                         |
|---------------------------------------------|---------------------------------------------------------------------------------------|
| Dynamically typed by default                | Define types once you figure out your data shapes                                     |
| Specified types can be disabled at any time | Compiler will use type knowledge to optimize codegen                                  |
|                                             | Types are specified in a malli/spec fashion, allowing robust and flexible definitions |
|                                             | Static typing maintains Clojure compatibility                                         |

## License
jank is under a strict copyleft license; see the
[LICENSE](https://github.com/jeaye/jank/blob/main/LICENSE) file.
