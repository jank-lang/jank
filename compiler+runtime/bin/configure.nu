#!/usr/bin/env nu

def --wrapped main [...args: string] {
    let here = $env.FILE_PWD
    let src_dir = ($here | path join "..")
    let build_dir = ($here | path join ".." "build")

    rm -rf ($build_dir | path join "CMakeCache.txt")
    rm -rf ($build_dir | path join "CMakeFiles")
    mkdir $build_dir

    # Default to clang, but respect existing CC/CXX env vars.
    let cc = ($env | get -i CC | default "clang")
    let cxx = ($env | get -i CXX | default "clang++")

    # Enable color diagnostics when stdout is a terminal.
    let color = try { term size; "ON" } catch { "OFF" }

    # On MSYS2, install to the MINGW prefix.
    let install_prefix = if (($env | get -i MSYSTEM | default "") != "") {
        [$"-DCMAKE_INSTALL_PREFIX=($env.MINGW_PREFIX)"]
    } else {
        []
    }

    with-env { CC: $cc, CXX: $cxx } {
        ^cmake ...$install_prefix -S $src_dir -B $build_dir $"-DCMAKE_COLOR_DIAGNOSTICS=($color)" ...$args
    }
}
