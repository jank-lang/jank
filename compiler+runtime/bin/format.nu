#!/usr/bin/env nu
use jank-mod.nu *

^git status
| lines
| where ($it =~ "modified:.*[hc]pp")
| each { |line|
    let file = ($line | str replace --regex ".*modified:\\s+" "")
    ^clang-format -i $file
    print $"formatted ($file)"
}

write-stamp "format.stamp" --profile ""
