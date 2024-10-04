{
  pkgs ? import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/28b5b8af91ffd2623e995e20aee56510db49001a.tar.gz";
    sha256 = "09zhy7bj0bd72r8dqpbrnpgapfkg5h91samrv1v8j0qxvv5kgv6n";
  }) { },
}:

let
  #stdenv = pkgs.llvm_19.stdenv;
  llvm = pkgs.llvm_19.override (oldAttrs: rec {
    version = "19.1.0";
    src = oldAttrs.src.overrideAttrs {
      outputHash = "";
    };
  });
  stdenv = llvm.stdenv;
  lib = pkgs.lib;
in

stdenv.mkDerivation {
  pname = "jank-lang";
  version = "dev";
  src = lib.cleanSource ./.;

  nativeBuildInputs = with pkgs; [
    llvm
    #llvmPackages_19.clang-unwrapped
    #llvmPackages_19.clangUseLLVM
    cmake
    git
    ninja
  ];

  buildInputs = with pkgs; [
    glibc
    llvmPackages_19.clang-unwrapped
    llvmPackages_19.clangUseLLVM
  ];

  CFLAGS = "-B${stdenv.cc.cc}/lib/gcc/${stdenv.targetPlatform.config}/${stdenv.cc.cc.version} -B${stdenv.cc.libc}/lib";

  cmakeFlags = [
    "-GNinja"
    "-DDMAKE_BUILD_TYPE=Debug"
    #"-DDCC_INSTALL_PREFIX=${pkgs.gcc}"
    "-Djank_tests=on"
  ];
}
