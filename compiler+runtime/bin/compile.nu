#!/usr/bin/env nu

def --wrapped main [
    --jobs (-j): int = 0  # Parallel jobs; 0 = half of logical CPUs rounded up
    ...args: string
] {
    let here = $env.FILE_PWD
    let build_dir = ($here | path dirname | path join "build")
    let j = if $jobs == 0 { (sys cpu | length) / 2 | math ceil } else { $jobs }
    ^cmake --build $build_dir --parallel $j ...$args

    let stamps_dir = ($here | path dirname | path join "stamps")
    mkdir $stamps_dir
    date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($stamps_dir | path join "build.stamp")
}
