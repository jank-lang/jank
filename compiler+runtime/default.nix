{
  pkgs ? import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/28b5b8af91ffd2623e995e20aee56510db49001a.tar.gz";
    sha256 = "09zhy7bj0bd72r8dqpbrnpgapfkg5h91samrv1v8j0qxvv5kgv6n";
  }) { },
}:

let
  stdenv = pkgs.llvmPackages_18.stdenv;
  lib = pkgs.lib;
in

stdenv.mkDerivation {
  pname = "jank-lang";
  version = "dev";
  src = lib.cleanSource ./.;

  nativeBuildInputs = with pkgs; [
    llvmPackages_18.llvm
    llvmPackages_18.clang-unwrapped
    llvmPackages_18.clangUseLLVM
    cmake
    git
  ];

  buildInputs = with pkgs; [
    glibc
  ];
}
