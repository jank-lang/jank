#!/usr/bin/env nu

def --wrapped main [...args: string] {
    let here = $env.FILE_PWD
    let build_dir = ($here | path join ".." "build")
    ^cmake --build $build_dir ...$args
}
