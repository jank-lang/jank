#!/usr/bin/env nu

^git status
| lines
| where ($it =~ "modified:.*[hc]pp")
| each { |line|
    let file = ($line | str replace --regex ".*modified:\\s+" "")
    ^clang-format -i $file
    print $"formatted ($file)"
}

let stamps_dir = ($env.FILE_PWD | path dirname | path join "stamps")
mkdir $stamps_dir
date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($stamps_dir | path join "format.stamp")
