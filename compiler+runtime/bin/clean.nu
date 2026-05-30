#!/usr/bin/env nu

let here = $env.FILE_PWD
let build_dir = ($here | path dirname | path join "build")
let stamps_dir = ($here | path dirname | path join "stamps")
let workspace = ($here | path dirname | path dirname)

# cmake-generated files that must be wiped for a clean reconfigure.
let cmake_state = [
    ($build_dir | path join "build.ninja")
    ($build_dir | path join "CMakeCache.txt")
    ($build_dir | path join "CMakeFiles")
    ($build_dir | path join "_deps")
]

if ($build_dir | path exists) {
    cd $build_dir
    if ("build.ninja" | path exists) {
        let result_ninja = (^ninja clean | complete)
        if $result_ninja.exit_code != 0 {
            # ninja clean failed — build system files are stale or broken.
            # Remove them so the next configure starts from scratch.
            print $"Warning: ninja clean exited ($result_ninja.exit_code) — removing stale cmake state"
        }
    } else if ("Makefile" | path exists) {
        let result_make = (^make clean | complete)
        if $result_make.exit_code != 0 {
            print $"Warning: make clean exited ($result_make.exit_code) — removing stale cmake state"
        }
    }
}

# Build a table of every path to remove, display it, then delete.
let to_remove = (
    $cmake_state
    | append [
        ($build_dir | path join "ar-cache"),
        ($build_dir | path join "core-libs"),
    ]
    | where { |p| $p | path exists }
    | each { |p| {kind: "build artifact", path: $p} }
    | append (
        glob ($stamps_dir | path join "*.stamp")
        | append (glob ($workspace | path join ".pixi" "*.stamp"))
        | append (glob ($workspace | path join ".pixi" "*-stamp"))
        | each { |f| {kind: "task stamp", path: $f} }
    )
)

$to_remove | table | print

$to_remove | each { |r| rm -rf $r.path } | ignore
