# runtime/internal-failure

This indicates an internal runtime problem or a low-level loading failure rather than a normal language mistake. Current uses include impossible loader states, missing internal artifacts, jar-reading failures, and unsupported generated arities.

## Mitigations
Try again from a clean state if the problem involves loading or compilation artifacts. If it still happens, report it as a jank bug with the exact command and input.
