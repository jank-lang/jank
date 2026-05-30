#!/usr/bin/env nu
use jank-mod.nu *

let workspace = ($env.FILE_PWD | path dirname | path dirname)

# Collect paths to remove and warn if the build system clean fails.
def collect-profile [profile: string] {
    let build_dir  = (jank-build-dir --profile $profile)
    let stamps_dir = (jank-stamps-dir --profile $profile)

    let cmake_state = [
        ($build_dir | path join "build.ninja")
        ($build_dir | path join "CMakeCache.txt")
        ($build_dir | path join "CMakeFiles")
        ($build_dir | path join "_deps")
    ]

    if ($build_dir | path exists) {
        if (($build_dir | path join "build.ninja") | path exists) {
            let result = (^ninja -C $build_dir clean | complete)
            if $result.exit_code != 0 {
                print $"Warning: ninja clean \(($profile)\) exited ($result.exit_code) — removing stale cmake state"
            }
        } else if (($build_dir | path join "Makefile") | path exists) {
            let result = (^make -C $build_dir clean | complete)
            if $result.exit_code != 0 {
                print $"Warning: make clean \(($profile)\) exited ($result.exit_code) — removing stale cmake state"
            }
        }
    }

    $cmake_state
    | append [
        ($build_dir | path join "ar-cache")
        ($build_dir | path join "core-libs")
    ]
    | where { |p| $p | path exists }
    | each { |p| {profile: $profile, kind: "build artifact", path: $p} }
    | append (
        glob ($stamps_dir | path join "*.stamp")
        | each { |f| {profile: $profile, kind: "task stamp", path: $f} }
    )
}

# Shared (non-profile) stamps.
let shared_stamps = (
    glob ($env.FILE_PWD | path dirname | path join "stamps" | path join "*.stamp")
    | append (glob ($workspace | path join ".pixi" "*.stamp"))
    | append (glob ($workspace | path join ".pixi" "*-stamp"))
    | each { |f| {profile: "shared", kind: "task stamp", path: $f} }
)

let to_remove = (
    (collect-profile "debug")
    | append (collect-profile "release")
    | append $shared_stamps
)

$to_remove | table | print

$to_remove | each { |r| rm -rf $r.path } | ignore
