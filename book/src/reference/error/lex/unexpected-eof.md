# lex/unexpected-eof
Unexpected end of file.

## Additional explanation
This is an uncommon error which generally indicates that the source file has
corrupt Unicode. Normal unterminated lists, strings, etc will result in
different errors.

## Mitigations
Verify the integrity of the file before proceeding.
