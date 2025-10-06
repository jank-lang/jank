{
  src,
  clang,
  cmake,
  gcc,
  lib,
  makeWrapper,
  ninja,
  python3,
  stdenv,
  targetPlatform,
  wrapCC,
  ...
}: let
  gccForLibs = stdenv.cc.cc;

  # nix adds these flags to the system clang transparently via a wrapped
  # executable. Since we will be compiling clang's runtime with our custom
  # (unwrapped) clang/clang++ executables, we need to provide them manually.
  runtimeCompileFlags = lib.strings.concatStringsSep " " [
    "-B${gccForLibs}/lib/gcc/${targetPlatform.config}/${gccForLibs.version}"
    "-B${stdenv.cc.libc}/lib"

    "-isystem ${gcc.cc}/include/c++/${gcc.version}"
    "-isystem ${gcc.cc}/include/c++/${gcc.version}/${targetPlatform.config}"
    "-isystem ${gcc.libc.dev}/include"

    # some of these flags won't be used all the time
    "-Wno-unused-command-line-argument"
  ];

  runtimeLinkFlags = lib.strings.concatStringsSep " " [
    "-L${gccForLibs}/lib"
    "-L${gccForLibs}/lib/gcc/${targetPlatform.config}/${gccForLibs.version}"
    "-L${stdenv.cc.libc}/lib"
  ];
in
  wrapCC (stdenv.mkDerivation {
    pname = "llvm-jank";
    version = "22.0.0-git";
    inherit src;

    nativeBuildInputs = [cmake ninja clang python3 makeWrapper];

    cmakeDir = "../llvm";
    cmakeFlags = [
      (lib.cmakeFeature "CMAKE_BUILD_TYPE" "Release")

      # use clang to compile clang
      (lib.cmakeFeature "CMAKE_C_COMPILER" "${clang}/bin/clang")
      (lib.cmakeFeature "CMAKE_CXX_COMPILER" "${clang}/bin/clang++")

      # from compiler+runtime/bin/build-clang
      (lib.cmakeBool "LLVM_BUILD_LLVM_DYLIB" true)
      (lib.cmakeBool "LLVM_LINK_LLVM_DYLIB" true)
      (lib.cmakeFeature "LLVM_ENABLE_PROJECTS" "clang")
      (lib.cmakeFeature "LLVM_TARGETS_TO_BUILD" "host")
      (lib.cmakeBool "LLVM_ENABLE_EH" true)
      (lib.cmakeBool "LLVM_ENABLE_RTTI" true)
      (lib.cmakeBool "LLVM_INCLUDE_BENCHMARKS" false)
      (lib.cmakeBool "LLVM_ENABLE_BINDINGS" false)
      (lib.cmakeBool "LLVM_INCLUDE_EXAMPLES" false)
      (lib.cmakeBool "LLVM_INCLUDE_TESTS" false)
      (lib.cmakeBool "LLVM_ENABLE_ZSTD" false)
      (lib.cmakeBool "LLVM_BUILD_OCAML_BINDINGS" false)
      (lib.cmakeBool "LLVM_ENABLE_OCAMLDOC" false)

      # nix-specific changes
      (lib.cmakeFeature "C_INCLUDE_DIRS" "${gcc.libc.dev}/include")
      (lib.cmakeFeature "LLVM_ENABLE_RUNTIMES" "libunwind;compiler-rt")
      # fix linking path for compiler-rt sanitize libs with
      # -fsanitize={address,undefined}
      (lib.cmakeBool "LLVM_ENABLE_PER_TARGET_RUNTIME_DIR" false)
      # libunwind shared library fails to compile, use static instead
      (lib.cmakeBool "LIBUNWIND_ENABLE_SHARED" false)
      (lib.cmakeBool "LIBUNWIND_ENABLE_STATIC" true)
    ];

    # see https://discourse.nixos.org/t/cmakeflags-and-spaces-in-option-values/20170
    preConfigure = ''
      cmakeFlagsArray+=(
        "-DLIBUNWIND_CXX_FLAGS=\"${runtimeCompileFlags}\""
        "-DLIBUNWIND_LINK_FLAGS=\"${runtimeLinkFlags}\""
      )
    '';

    passthru.isClang = true;
  })
