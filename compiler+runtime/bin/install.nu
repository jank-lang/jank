#!/usr/bin/env nu
use jank-mod.nu *

def --wrapped main [
    --profile: string = "release"  # Build profile to install from; defaults to "release"
    ...args: string
] {
    ^cmake --install (jank-build-dir --profile $profile) ...$args
    write-stamp "install.stamp" --profile $profile
}
