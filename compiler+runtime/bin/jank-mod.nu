# Shared build utilities for jank's compiler+runtime build scripts.
# Import with: use jank-mod.nu *
#
# Profiles: "debug" (Debug build, default) or "release" (Release build).
# Pass --profile "" to write-stamp / jank-stamps-dir for non-profile-specific stamps.

# Absolute path to the cmake build directory for the given profile.
export def jank-build-dir [--profile: string = "debug"] {
    $env.FILE_PWD | path dirname | path join $"build-($profile)"
}

# Absolute path to the task stamp directory.
# Pass --profile "" for stamps shared across profiles (lint, format, sync…).
export def jank-stamps-dir [--profile: string = "debug"] {
    let base = ($env.FILE_PWD | path dirname | path join "stamps")
    if $profile == "" { $base } else { $base | path join $profile }
}

# Write a completion stamp with the current timestamp.
# Use --profile "" for stamps that are not build-profile-specific.
export def write-stamp [name: string, --profile: string = "debug"] {
    let sd = (jank-stamps-dir --profile $profile)
    mkdir $sd
    date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($sd | path join $name)
}

# Configure the cmake build.  Extra cmake -D flags may be passed as arguments.
# Automatically sets -DCMAKE_BUILD_TYPE from the profile unless the caller overrides it.
export def --wrapped configure [
    --profile: string = "debug"
    ...args: string
] {
    let src_dir   = ($env.FILE_PWD | path dirname)
    let build_dir = (jank-build-dir --profile $profile)
    mkdir $build_dir

    # Default to clang/clang++ on Unix, clang-cl on Windows (MSVC ABI).
    let is_windows = ($nu.os-info.name == "windows")
    let cc  = ($env | get -o CC  | default (if $is_windows { "clang-cl" } else { "clang"   }))
    let cxx = ($env | get -o CXX | default (if $is_windows { "clang-cl" } else { "clang++" }))

    let color = try { term size; "ON" } catch { "OFF" }

    # When running inside pixi/conda, expose the environment prefix to cmake.
    let prefix_flags = match ($env | get -o CONDA_PREFIX) {
        null => []
        $p   => [$"-DCMAKE_PREFIX_PATH=($p)"]
    }

    # Default to Ninja unless the caller already passed a -G flag.
    let generator_flags = if ($args | any { |a| $a | str starts-with "-G" }) { [] } else { ["-GNinja"] }

    # Derive CMAKE_BUILD_TYPE from the profile unless the caller already set it.
    let profile_flags = if ($args | any { |a| $a | str starts-with "-DCMAKE_BUILD_TYPE" }) {
        []
    } else {
        match $profile {
            "debug"   => ["-DCMAKE_BUILD_TYPE=Debug"]
            "release" => ["-DCMAKE_BUILD_TYPE=Release"]
            _         => []
        }
    }

    with-env { CC: $cc, CXX: $cxx } {
        ^cmake ...$prefix_flags ...$generator_flags ...$profile_flags -S $src_dir -B $build_dir $"-DCMAKE_COLOR_DIAGNOSTICS=($color)" ...$args
    }

    write-stamp "configure.stamp" --profile $profile
}

# Build jank.  Defaults to half of logical CPUs; override with --jobs/-j.
export def --wrapped compile [
    --profile: string = "debug"
    --jobs (-j): int = 0  # Parallel jobs; 0 = half of logical CPUs rounded up
    ...args: string
] {
    let build_dir = (jank-build-dir --profile $profile)
    let j = if $jobs == 0 { (sys cpu | length) / 2 | math ceil } else { $jobs }
    ^cmake --build $build_dir --parallel $j ...$args
    write-stamp "build.stamp" --profile $profile
}
