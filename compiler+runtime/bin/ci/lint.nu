#!/usr/bin/env nu

# Syntax-check all nushell scripts under bin/.
glob "bin/**/*.nu" | each { |f|
    print $"Checking ($f)"
    ^nu --check $f
}
