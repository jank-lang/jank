{
  pkgs ? import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/75b209227dff3cbfac19f510a62f9446c92beac4.tar.gz";
    sha256 = "166varzdamn2kkw5zslarzl6026q87wjnss1hlhdsq89hwvfgd2d";
  }) { },
}:

let
  stdenv = pkgs.stdenv;
  cc = stdenv.cc.cc;
in

stdenv.mkDerivation rec {
  pname = "boehm-gc";
  version = "8.2.8";

  src = pkgs.fetchFromGitHub {
    owner = "ivmai";
    repo = "bdwgc";
    rev = "v${version}";
    hash = "sha256-UQSLK/05uPal6/m+HMz0QwXVII1leonlmtSZsXjJ+/c=";
  };

  nativeBuildInputs = [
    pkgs.cmake
  ];
}
