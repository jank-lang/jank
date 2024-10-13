{
  pkgs ? import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/75b209227dff3cbfac19f510a62f9446c92beac4.tar.gz";
    sha256 = "166varzdamn2kkw5zslarzl6026q87wjnss1hlhdsq89hwvfgd2d";
  }) { },
}:

let
  stdenv = pkgs.llvmPackages_19.stdenv;
  cc = stdenv.cc.cc;
in

stdenv.mkDerivation rec {
  pname = "libzippp";
  version = "7.1-1.10.1";

  src = pkgs.fetchFromGitHub {
    owner = "ctabin";
    repo = pname;
    rev = "${pname}-v${version}";
    hash = "sha256-ffX4UuDKMgSYwIecmJnj+XLnjsMwUbK6rraOk0z4Ma8=";
  };

  nativeBuildInputs = [
    pkgs.cmake
  ];

  buildInputs = [
    pkgs.zlib
    pkgs.libzip
  ];
}
