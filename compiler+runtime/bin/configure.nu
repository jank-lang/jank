#!/usr/bin/env nu
use jank-mod.nu *

def --wrapped main [
    --profile: string = "debug"  # Build profile: "debug" or "release"
    ...args: string
] {
    configure --profile $profile ...$args
}
