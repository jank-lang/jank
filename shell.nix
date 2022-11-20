with (import <nixpkgs> {});
let pkgsUnstable = import
(
  fetchTarball https://github.com/NixOS/nixpkgs-channels/archive/nixos-unstable.tar.gz
)
{ };
in
mkShell
{
  buildInputs =
  [
    # Build deps.
    cmake
    ninja
    clang

    # Dev tools.
    llvm # For clang-format
    entr
    gcovr
    lcov
    git
    shellcheck
  ];
  shellHook =
  ''
  export CC="${pkgs.clang}/bin/clang"
  export CXX="${pkgs.clang}/bin/clang++"
  '';
}
