# system/clang-executable-not-found

This happens when jank cannot find a suitable Clang executable with the required major version.

## Mitigations
jank should be installed with its own Clang version, so this error indicates an
issue with the jank install. The first step to troubleshoot this would be a
[health check](/troubleshooting/health-check.md).
