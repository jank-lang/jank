with (import <nixpkgs> {});
let pkgsUnstable = import
(
  fetchTarball https://github.com/NixOS/nixpkgs-channels/archive/nixos-unstable.tar.gz
);
#local-cling = callPackage ./cling.nix { };
in
mkShell
{
  buildInputs =
  [
    # Build deps.
    cmake
    ninja
    pkg-config
    clang
    libffi
    libxcrypt

    #local-cling

    # Dev tools.
    entr
    gcovr
    lcov
    git
    shellcheck
    # For clangd
    llvm
  ];
  shellHook =
  ''
  export CC="${pkgs.clang}/bin/clang"
  export CXX="${pkgs.clang}/bin/clang++"
  '';
}
