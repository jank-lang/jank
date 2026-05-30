#!/usr/bin/env nu

# CMake copies this script into the build directory during configuration,
# substituting @VARIABLE@ markers with their values.
let ar = "@CMAKE_AR@"
let cmake_binary_dir = "@CMAKE_CURRENT_BINARY_DIR@"
let cache_dir = "@CMAKE_CURRENT_BINARY_DIR@/ar-cache"

def collect-object-files [] {
    ls $cache_dir
    | where type == file
    | get name
    | each { |f| open --raw $f | lines | where { |l| ($l | str trim) != "" } }
    | flatten
}

def --wrapped main [command: string, ...args: string] {
    mkdir $cache_dir

    match $command {
        # Called from bin/clean.nu — not normally needed.
        "clean" => {
            rm -rf $cache_dir
            mkdir $cache_dir
        }

        # Merge all cached object files into libjank-standalone-phase-1.a.
        # Run prior to building the jank executable, after all libraries are built.
        "merge" => {
            let object_files = collect-object-files
            let merge_output = ($cmake_binary_dir | path join "libjank-standalone-phase-1.a")
            if ($merge_output | path exists) { rm -f $merge_output }
            run-external $ar "qc" $merge_output ...$object_files
        }

        # Like merge, but also bundles Clojure core library object files for
        # the phase-2 binary.
        "merge-phase-2" => {
            let base_objects = collect-object-files
            let extra_objects = (
                "@jank_clojure_core_binary_output@ @jank_nrepl_server_outputs_str@"
                | split row --regex '\s+'
                | where { |s| ($s | str trim) != "" }
            )
            let all_files = ($base_objects | append $extra_objects)
            let merge_output = ($cmake_binary_dir | path join "libjank-standalone.a")
            if ($merge_output | path exists) { rm -f $merge_output }
            run-external $ar "qc" $merge_output ...$all_files
        }

        # Default: delegate to the real AR and record the object files for later merging.
        _ => {
            let output = ($args | first)
            let remaining = ($args | skip 1)
            let base = ($output | str replace --all "/" "_")
            let list_file = ($cache_dir | path join $"($base).list")
            if ($list_file | path exists) { rm -f $list_file }
            # One path per line — avoids word-boundary splitting on '/' when reading back.
            $remaining | str join "\n" | save -f $list_file
            run-external $ar $command $output ...$remaining
        }
    }
}
