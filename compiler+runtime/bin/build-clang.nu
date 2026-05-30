#!/usr/bin/env nu

def main [
    --update (-u)         # Update existing LLVM source to the tracked branch
    --jobs (-j): int = 0  # Parallel jobs (default: number of logical CPUs)
] {
    let here = $env.FILE_PWD
    let build_dir = ($here | path dirname | path join "build")
    mkdir $build_dir
    cd $build_dir

    let make_j = if $jobs == 0 { sys cpu | length } else { $jobs }
    print $"Using ($make_j) cores to build"

    let srcdir = $env.PWD

    let llvm_url = "https://github.com/jank-lang/llvm-project.git"
    let llvm_version = 22
    let llvm_branch = "jank"
    let llvm_commit = "8164f1a0c17b192e133817436bdb07598b7402a3"
    let llvm_dest_dir = ($srcdir | path join "llvm-install")
    let llvm_build_dir = ($srcdir | path join "llvm-build")
    let llvm_src_dir = ($srcdir | path join "llvm")

    # --- prepare ---
    if not ($llvm_src_dir | path exists) {
        ^git clone --no-checkout --filter=blob:none --depth 1 $llvm_url $llvm_src_dir
        cd $llvm_src_dir
        ^git fetch origin $llvm_commit --depth 1
        ^git checkout $llvm_commit
        ^git submodule update --init --depth 1 --recursive
        cd $build_dir
    } else if $update {
        cd $llvm_src_dir
        try { ^git stash | ignore } catch { }
        ^git checkout $llvm_branch
        ^git pull origin $llvm_branch
        cd $build_dir
    }

    mkdir $llvm_build_dir
    cd $llvm_build_dir

    let cmake_args = [
        "-DCMAKE_BUILD_TYPE=Release"
        "-DLLVM_ENABLE_RUNTIMES=libcxx;libcxxabi;libunwind;compiler-rt"
        "-DLLVM_BUILD_LLVM_DYLIB=ON"
        "-DLLVM_LINK_LLVM_DYLIB=ON"
        "-DLLVM_ENABLE_PROJECTS=clang;clang-tools-extra"
        "-DLLVM_TARGETS_TO_BUILD=host"
        "-DLLVM_ENABLE_EH=ON"
        "-DLLVM_ENABLE_RTTI=ON"
        "-DLLVM_INCLUDE_BENCHMARKS=OFF"
        "-DLLVM_ENABLE_BINDINGS=OFF"
        "-DLLVM_INCLUDE_EXAMPLES=OFF"
        "-DLLVM_INCLUDE_TESTS=OFF"
        "-DLLVM_ENABLE_ZSTD=OFF"
        "-DLLVM_BUILD_OCAML_BINDINGS=OFF"
        "-DLLVM_ENABLE_OCAMLDOC=OFF"
        "-G" "Ninja"
    ]
    ^cmake ...$cmake_args ($llvm_src_dir | path join "llvm")

    with-env { DESTDIR: $llvm_dest_dir } {
        ^ninja clang clang-repl compiler-rt clang-cpp clang-tidy install $"-j($make_j)"
    }

    let clang_lib_src = ($llvm_build_dir | path join $"lib/clang/($llvm_version)/lib")
    let clang_lib_dst = ($llvm_dest_dir | path join $"usr/local/lib/clang/($llvm_version)/lib")
    mkdir $clang_lib_dst
    ls $clang_lib_src | get name | each { |item| cp -r $item $clang_lib_dst }

    print ""
    print "Clang/LLVM successfully compiled. You can now build jank."

    let stamps_dir = ($here | path dirname | path join "stamps")
    mkdir $stamps_dir
    date now | format date "%Y-%m-%dT%H:%M:%S%z" | save --force ($stamps_dir | path join "build-clang.stamp")
}
