#!/usr/bin/env nu

def --wrapped main [...args: string] {
    ^llvm-cov gcov ...$args
}
