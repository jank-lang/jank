#!/usr/bin/env nu

def --wrapped main [...args: string] {
    let here = $env.FILE_PWD
    let src_dir = ($here | path dirname)
    let build_dir = ($here | path dirname | path join "build")

    mkdir $build_dir

    # Default to clang/clang++ on Unix, clang-cl on Windows (MSVC ABI).
    let is_windows = ($nu.os-info.name == "windows")
    let cc  = ($env | get -o CC  | default (if $is_windows { "clang-cl" } else { "clang"   }))
    let cxx = ($env | get -o CXX | default (if $is_windows { "clang-cl" } else { "clang++" }))

    # Enable color diagnostics when stdout is a terminal.
    let color = try { term size; "ON" } catch { "OFF" }

    # When running inside pixi/conda, expose the environment prefix to cmake
    # so that find_package() locates pixi-managed libraries (Boost, OpenSSL, …).
    let prefix_flags = match ($env | get -o CONDA_PREFIX) {
        null => []
        $p   => [$"-DCMAKE_PREFIX_PATH=($p)"]
    }

    # Default to Ninja; only skip if the caller already passed a -G flag.
    let generator_flags = if ($args | any { |a| $a | str starts-with "-G" }) { [] } else { ["-GNinja"] }

    with-env { CC: $cc, CXX: $cxx } {
        ^cmake ...$prefix_flags ...$generator_flags -S $src_dir -B $build_dir $"-DCMAKE_COLOR_DIAGNOSTICS=($color)" ...$args
    }

    let stamps_dir = ($here | path dirname | path join "stamps")
    mkdir $stamps_dir
    date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($stamps_dir | path join "configure.stamp")
}
