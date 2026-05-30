#!/usr/bin/env nu

# Syntax-check all nushell scripts under bin/.
glob "bin/**/*.nu" | each { |f|
    print $"Checking ($f)"
    ^nu --ide-check 100 $f
}

use ../jank-mod.nu *
write-stamp "lint.stamp" --profile ""
