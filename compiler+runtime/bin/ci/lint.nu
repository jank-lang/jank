#!/usr/bin/env nu

# Syntax-check all nushell scripts under bin/.
glob "bin/**/*.nu" | each { |f|
    print $"Checking ($f)"
    ^nu --ide-check 100 $f
}

let stamps_dir = ($env.FILE_PWD | path dirname | path dirname | path join "stamps")
mkdir $stamps_dir
date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($stamps_dir | path join "lint.stamp")
