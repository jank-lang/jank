# Printing jank's IR or codegen
You can glimpse under the hood of jank and have it print out both the generated
jank IR and the generated C++ code, separately. These are accomplished via
environment variables.

* `JANK_PRINT_IR=1` will print out formatted IR for each compiled jank function.
* `JANK_PRINT_CODEGEN=1` will print out formatted C++ code for each compiled function.

Note that not all evaluated code is compiled. If you want to be sure some code
is compiled, wrap it in a function and call it.
