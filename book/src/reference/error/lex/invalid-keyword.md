# lex/invalid-keyword
Invalid keyword.

## Additional explanation
This happens when a keyword literal is missing its name or has an invalid namespace/name shape. Common cases include `:`, `::`, `:/foo`, `::/foo`, or too many leading colons.

## Mitigations
Write the keyword as `:name`, `:ns/name`, `::name`, or `::alias/name`.
