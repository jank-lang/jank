# lex/expecting-whitespace

This happens when two forms that must be separated are written back to back without whitespace.

## Mitigations
Add a space or newline between the two forms. If they are meant to be one token, rewrite them as a single valid token.
