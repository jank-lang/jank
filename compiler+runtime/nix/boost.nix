{
  pkgs,
}:

pkgs.stdenv.mkDerivation rec {
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
