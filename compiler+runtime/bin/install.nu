#!/usr/bin/env nu

def --wrapped main [...args: string] {
    let here = $env.FILE_PWD
    ^cmake --install build ...$args

    let stamps_dir = ($here | path dirname | path join "stamps")
    mkdir $stamps_dir
    date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($stamps_dir | path join "install.stamp")
}
