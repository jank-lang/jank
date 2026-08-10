# parse/invalid-reader-tag-value

This happens when a tagged literal is missing its following form. It can also happen when the following form has the wrong kind for that tag.

## Mitigations
Place a valid form immediately after the tag. Make sure it matches what the tag expects.
