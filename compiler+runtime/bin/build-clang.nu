#!/usr/bin/env nu

def main [
    --install (-i)        # Install binary packages (MSYS2 only)
    --update (-u)         # Update existing LLVM source to the tracked branch
    --jobs (-j): int = 0  # Parallel jobs (default: number of logical CPUs)
] {
    let here = $env.FILE_PWD
    let build_dir = ($here | path join ".." "build")
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

    let is_msys2 = (($env | get -i MSYSTEM | default "") != "")

    let llvm_src_dir = if $is_msys2 {
        $llvm_build_dir | path join "src"
    } else {
        $srcdir | path join "llvm"
    }

    let llvm_mingw_pkg_gen = 1
    let llvm_mingw_pkg_url = "https://github.com/jank-lang/MINGW-packages"
    let llvm_mingw_pkg_commit = "70ffe41c3d45164a497b7cac65448057dcebf4ae"
    let llvm_mingw_pkg_repo_dir = ($build_dir | path join "MINGW-packages")
    let llvm_mingw_pkg_build_dir = $llvm_build_dir

    if $install {
        if $is_msys2 {
            glob ($llvm_dest_dir | path join "*.tar.zst") | each { |pkg|
                ^pacman --noconfirm -U $pkg
            }
        } else {
            error make { msg: "Binary package installation is only supported in the MSYS2 CLANG environment on Windows." }
        }
        return
    }

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

    if $is_msys2 {
        ^pacman --noconfirm -S base-devel git

        print $"Cloning MINGW-packages repo to ($llvm_mingw_pkg_repo_dir)..."
        rm -rf $llvm_mingw_pkg_repo_dir
        ^git clone --depth 1 --filter=blob:none --no-checkout $llvm_mingw_pkg_url $llvm_mingw_pkg_repo_dir
        cd $llvm_mingw_pkg_repo_dir
        ^git sparse-checkout init --cone
        ^git sparse-checkout set mingw-w64-llvm
        ^git checkout $llvm_mingw_pkg_commit
        cd $build_dir

        ls ($llvm_mingw_pkg_repo_dir | path join "mingw-w64-llvm") | get name | each { |f|
            cp $f $llvm_mingw_pkg_build_dir
        }

        let llvm_git_hash = (^git -C $llvm_src_dir rev-parse --short HEAD | str trim)
        let pkgbuild = ($llvm_mingw_pkg_build_dir | path join "PKGBUILD")
        open $pkgbuild
        | str replace --regex "(?m)^pkgrel=0\\.jank$" $"pkgrel=($llvm_mingw_pkg_gen).jank_($llvm_git_hash)"
        | save -f $pkgbuild
    }

    # --- build ---
    if $is_msys2 {
        cd $llvm_build_dir
        ^makepkg-mingw -sLf --nocheck
        rm -rf $llvm_dest_dir
        mkdir $llvm_dest_dir
        glob ($llvm_build_dir | path join "*.tar.zst") | each { |f| mv $f $llvm_dest_dir }
        print $"LLVM binary packages copied to ($llvm_dest_dir)."
    } else {
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
    }

    print ""
    print "Clang/LLVM successfully compiled. You can now build jank."
}
