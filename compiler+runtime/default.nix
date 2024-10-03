{
  pkgs ? import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/28b5b8af91ffd2623e995e20aee56510db49001a.tar.gz";
    sha256 = "09zhy7bj0bd72r8dqpbrnpgapfkg5h91samrv1v8j0qxvv5kgv6n";
  }) { },
}:

let
  stdenv = pkgs.clangStdenv;
  lib = pkgs.lib;
in

stdenv.mkDerivation {
  pname = "jank-lang";
  version = "dev";
  src = lib.cleanSource ./.;
}
