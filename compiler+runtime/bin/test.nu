#!/usr/bin/env nu
use jank-mod.nu *

def --wrapped main [...args: string] {
    # Tests always use the debug profile with tests enabled.
    configure --profile debug -Djank_test=on

    let build_dir = (jank-build-dir --profile debug)

    # Remove any previous code coverage files.
    glob $"($build_dir)/jank-*.profraw" | each { |f| rm -f $f }
    let profdata = ($build_dir | path join "jank.profdata")
    if ($profdata | path exists) { rm -f $profdata }

    compile --profile debug

    let test_bin = ($build_dir | path join "jank-test")
    ^$test_bin ...$args

    write-stamp "test.stamp" --profile debug
}
