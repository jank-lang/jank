#!/usr/bin/env nu

def --wrapped main [...cmd: string] {
    # Sleep before running accounts for saving multiple files simultaneously.
    # Without the sleep we may start compiling before all saves complete, which
    # can produce false results and hard-to-debug issues.
    let cmd_str = ($cmd | str join " ")
    ^git ls-files -cdmo --exclude-standard | ^entr nu -c $"sleep 1; ($cmd_str)"
}
