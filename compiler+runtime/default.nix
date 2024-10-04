{
  pkgs ? import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/75b209227dff3cbfac19f510a62f9446c92beac4.tar.gz";
    sha256 = "166varzdamn2kkw5zslarzl6026q87wjnss1hlhdsq89hwvfgd2d";
  }) { },
}:

let
  llvm = pkgs.llvm_19;
  stdenv = llvm.stdenv;
  lib = pkgs.lib;

  # Deps
  boost = pkgs.callPackage ./nix/boost.nix { };
  boehmgc = pkgs.callPackage ./nix/boehmgc.nix { };
  libzippp = pkgs.callPackage ./nix/libzippp.nix { };
in

stdenv.mkDerivation {
  pname = "jank-lang";
  version = "dev";
  src = lib.cleanSource ./.;

  nativeBuildInputs = with pkgs; [
    llvm
    cmake
    git
    ninja
  ];

  buildInputs = [
    boost
    boehmgc
    libzippp
  ] ++ (with pkgs; [
    openssl
    doctest
    double-conversion
    readline
    libzip
    immer
    cli11
    magic-enum
    fmt
    llvmPackages_19.clang-unwrapped
    llvmPackages_19.clangUseLLVM
  ]);

  cmakeFlags = [
    "-GNinja"
    "-DDMAKE_BUILD_TYPE=Debug"
    "-Djank_tests=on"
  ];
}
