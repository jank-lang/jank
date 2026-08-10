# parse/invalid-meta-hint-target

This happens when metadata is attached to a missing form or to a value that cannot carry metadata. Some values, such as numbers and strings, do not support metadata.

## Mitigations
Put the metadata before a valid target form that supports metadata. If needed, wrap the value in something that can carry metadata, such as an atom or a container.
