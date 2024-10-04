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
  pname = "boost";
  version = "1.86.0";

  src = pkgs.fetchgit {
    url = "https://github.com/boostorg/boost";
    rev = "boost-1.86.0";
    hash = "sha256-8Ra5VNQUpd29u3oHMmP6V2JQtvJJz2D+E33HPi+L6uA=";
    fetchSubmodules = true;
  };

  nativeBuildInputs = [
    pkgs.cmake
  ];
}
