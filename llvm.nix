{
  stdenv,
  lib,
  pkgs,
  ...
}: let
  gccForLibs = stdenv.cc.cc;

  # nix adds these flags to the system clang transparently via a wrapped
  # executable. Since we will be compiling clang's runtime with our custom
  # (unwrapped) clang/clang++ executables, we need to provide them manually.
  runtimeCompileFlags = lib.strings.concatStringsSep " " [
    "-B${gccForLibs}/lib/gcc/${pkgs.targetPlatform.config}/${gccForLibs.version}"
    "-B${stdenv.cc.libc}/lib"

    "-isystem ${pkgs.gcc.cc}/include/c++/${pkgs.gcc.version}"
    "-isystem ${pkgs.gcc.cc}/include/c++/${pkgs.gcc.version}/${pkgs.targetPlatform.config}"
    "-isystem ${pkgs.gcc.libc.dev}/include"

    # some of these flags won't be used all the time
    "-Wno-unused-command-line-argument"
  ];

  runtimeLinkFlags = lib.strings.concatStringsSep " " [
    "-L${gccForLibs}/lib"
    "-L${gccForLibs}/lib/gcc/${pkgs.targetPlatform.config}/${gccForLibs.version}"
    "-L${stdenv.cc.libc}/lib"
  ];
in
  stdenv.mkDerivation {
    pname = "llvm-jank";
    version = "21.0.0-git";

    src = pkgs.fetchFromGitHub {
      owner = "jank-lang";
      repo = "llvm-project";
      rev = "3bfa007f9db853ddeb9ed88c27e443e781e53790";
      sha256 = "sha256-hLTSk0eH2gNAo3Mml3tbBL4o00KfkmQRI+bqRgKxG9U=";
    };

    nativeBuildInputs = [pkgs.cmake pkgs.ninja pkgs.clang pkgs.python3 pkgs.makeWrapper];

    cmakeDir = "../llvm";
    cmakeFlags = [
      (lib.cmakeFeature "CMAKE_BUILD_TYPE" "Release")

      # use clang to compile clang
      (lib.cmakeFeature "CMAKE_C_COMPILER" "${pkgs.clang}/bin/clang")
      (lib.cmakeFeature "CMAKE_CXX_COMPILER" "${pkgs.clang}/bin/clang++")

      # from compiler+runtime/bin/build-clang
      (lib.cmakeBool "LLVM_BUILD_LLVM_DYLIB" true)
      (lib.cmakeBool "LLVM_LINK_LLVM_DYLIB" true)
      (lib.cmakeFeature "LLVM_ENABLE_PROJECTS" "clang;compiler-rt")
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
      (lib.cmakeFeature "C_INCLUDE_DIRS" "${pkgs.gcc.libc.dev}/include")
      (lib.cmakeFeature "LLVM_ENABLE_RUNTIMES" "libunwind")
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

    # Create wrapped clang/clang++ executables for compiling the jank compiler,
    # and for in turn compiling jank code.
    postInstall = ''
      wrapProgram $out/bin/clang --add-flags "${runtimeCompileFlags + " " + runtimeLinkFlags}"
      wrapProgram $out/bin/clang++ --add-flags "${runtimeCompileFlags + " " + runtimeLinkFlags}"
    '';
  }
