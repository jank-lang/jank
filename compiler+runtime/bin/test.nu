#!/usr/bin/env nu

def --wrapped main [...args: string] {
    let here = $env.FILE_PWD
    let build_dir = ($here | path dirname | path join "build")

    # Remove any previous code coverage files.
    glob $"($build_dir)/jank-*.profraw" | each { |f| rm -f $f }
    let profdata = ($build_dir | path join "jank.profdata")
    if ($profdata | path exists) { rm -f $profdata }

    ^nu ($here | path join "compile.nu")
    let test_bin = ($build_dir | path join "jank-test")
    ^$test_bin ...$args

    let stamps_dir = ($here | path dirname | path join "stamps")
    mkdir $stamps_dir
    date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($stamps_dir | path join "test.stamp")
}
