# lex/incomplete-character

This happens when a character literal starts but no complete character value follows.

## Mitigations
Complete the character literal. Examples include `\a`, `\space`, `\newline`. Otherwise remove the partial literal.
