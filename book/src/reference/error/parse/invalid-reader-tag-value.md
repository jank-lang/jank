# parse/invalid-reader-tag-value
Invalid reader tag value.

## Additional explanation
This happens when a tagged literal is missing its following form, or when the following form has the wrong kind for that tag.

## Mitigations
Place a valid form immediately after the tag, and make sure it matches what that tag expects.
