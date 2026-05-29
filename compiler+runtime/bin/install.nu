#!/usr/bin/env nu

def --wrapped main [...args: string] {
    ^cmake --install build ...$args
}
