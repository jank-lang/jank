#!/usr/bin/env nu

let here = $env.FILE_PWD
let build_dir = ($here | path join ".." "build")

if ($build_dir | path exists) {
    cd $build_dir
    if ("build.ninja" | path exists) {
        ^ninja clean
    } else if ("Makefile" | path exists) {
        ^make clean
    }
    rm -rf ($build_dir | path join "ar-cache")
    rm -rf ($build_dir | path join "core-libs")
}
