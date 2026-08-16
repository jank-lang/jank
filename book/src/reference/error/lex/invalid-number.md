# lex/invalid-number

This happens when a numeric literal is malformed. Common causes include mixed number syntax. They also include digits that do not match the base. Another cause is an unsupported base. Another cause is an unfinished literal.

## Mitigations
Rewrite the literal using one valid number format. Make sure every required digit is present.
