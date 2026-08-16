# runtime/uncaught-exception
This error happens when an exception, of some kind, was thrown and then not
caught by your program's code. The jank runtime caught the exception and then
surfaced the stack trace, along with the exception message.

## Mitigations
Inspect the stack trace to find where the exception was thrown and whether or
not that was intended. If the exception was intended, add a `try` to your
program so that you can catch the exception yourself.
