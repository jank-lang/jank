# lex/invalid-keyword

This happens when a keyword literal is missing its name or has an invalid namespace or name shape. Common cases include `:`, `::`, `:/foo`, `::/foo`. Another common case is too many leading colons.

## Mitigations
Use one of these shapes: `:name`, `:ns/name`, `::name`, `::alias/name`.
