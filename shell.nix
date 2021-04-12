{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation
{
  name = "jank";
  buildInputs = with pkgs;
  [
    boost
  ];
}
