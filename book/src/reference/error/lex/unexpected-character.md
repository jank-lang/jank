# lex/unexpected-character
Unexpected character.

## Additional explanation
This happens when the source contains a character that cannot start any valid jank token in that position.

## Mitigations
Remove or replace the stray character, or put it inside a string or other valid form if that was your intent.
