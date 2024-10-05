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
  pname = "openssl";
  version = "3.3.2";

  src = pkgs.fetchFromGitHub {
    owner = pname;
    repo = pname;
    rev = "${pname}-${version}";
    hash = "sha256-3KB0fetgXloCniFsvzzuchKgopPdQdh9/00M1mqJWyg=";
  };

  buildInputs = [
    pkgs.perl
  ];

  configurePhase = ''
    perl ./Configure --prefix=$out no-shared
  '';

  buildPhase = ''
    make
  '';

  installPhase = ''
    make install
  '';
}
