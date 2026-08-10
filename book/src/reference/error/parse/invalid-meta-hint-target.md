# parse/invalid-meta-hint-target
Invalid meta hint target.

## Additional explanation
This happens when metadata is attached to a missing form or to a value that
cannot carry metadata. Note that some objects, like numbers and strings, don't
support metadata.

## Mitigations
Put the metadata before a valid target form that supports metadata. If needed,
wrap the value in something which does support metadata, like an atom or a
container.
