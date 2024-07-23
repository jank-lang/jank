with (import <nixpkgs> {});
let pkgsUnstable = import
(
  fetchTarball https://github.com/NixOS/nixpkgs-channels/archive/nixos-unstable.tar.gz
);
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

    # Dev tools.
    entr
    gcovr
    lcov
    git
    shellcheck
    # For clangd
    llvm

    # Libs.
    boehmgc
    # TODO: CMake fails to find boost::preprocessor.
    boost
    cli11
    fmt
    immer
    # TODO: Doesn't have a nix pkg.
    #libzippp
    magic-enum
    readline
  ];
  shellHook =
  ''
  export CC="${pkgs.clang}/bin/clang"
  export CXX="${pkgs.clang}/bin/clang++"
  '';
}
