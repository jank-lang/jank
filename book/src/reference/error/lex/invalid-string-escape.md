# lex/invalid-string-escape
Invalid string escape sequence.

## Additional explanation
This error is for a string escape sequence that jank does not recognize, or for a malformed Unicode escape inside a string literal.

## Mitigations
Use a supported escape sequence such as `\n`, `\t`, `\\`, `\"`, or a valid `\uXXXX` escape.
