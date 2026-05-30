#!/usr/bin/env nu

^git submodule update --init --recursive

let stamps_dir = ($env.FILE_PWD | path dirname | path join "compiler+runtime" "stamps")
mkdir $stamps_dir
date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($stamps_dir | path join "sync.stamp")
