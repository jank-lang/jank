#!/usr/bin/env nu
use jank-mod.nu *

def --wrapped main [
    --profile: string = "debug"  # Build profile: "debug" or "release"
    --jobs (-j): int = 0         # Parallel jobs; 0 = half of logical CPUs rounded up
    ...args: string
] {
    compile --profile $profile --jobs $jobs ...$args
}
