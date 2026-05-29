#!/usr/bin/env nu

let original_dir = $env.PWD
let tmp_dir = (^mktemp -d | str trim)

cd $tmp_dir
^git init out+err> /dev/null
touch empty
^git add empty
^git commit -m "Add empty" out+err> /dev/null

let sig_lines = (^git log --show-signature | lines | where ($it | str contains "gpg:"))
if ($sig_lines | is-empty) {
    print "GPG signing is not working"
} else {
    $sig_lines | each { |line| print $line }
}

cd $original_dir
rm -rf $tmp_dir
