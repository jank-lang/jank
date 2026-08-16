# parse/invalid-string-escape
This happens when jank does not recognize a string escape sequence. It can also
happen for a malformed Unicode escape inside a string literal.

## Mitigations
Use a supported escape sequence such as `\n`, `\t`, `\\`, `\"`. You can also use
a valid `\uXXXX` escape.
