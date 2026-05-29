#!/usr/bin/env nu

def --wrapped main [...args: string] {
    let here = $env.FILE_PWD
    let build_dir = ($here | path join ".." "build")

    # Remove any previous code coverage files.
    glob $"($build_dir)/jank-*.profraw" | each { |f| rm -f $f }
    let profdata = ($build_dir | path join "jank.profdata")
    if ($profdata | path exists) { rm -f $profdata }

    ^nu ($here | path join "compile.nu")
    let test_bin = ($build_dir | path join "jank-test")
    ^$test_bin ...$args
}
