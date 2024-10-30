{
  pkgs ? import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/28b5b8af91ffd2623e995e20aee56510db49001a.tar.gz";
    sha256 = "09zhy7bj0bd72r8dqpbrnpgapfkg5h91samrv1v8j0qxvv5kgv6n";
  }) { },
}:

pkgs.mkShell {
  buildInputs = with pkgs; [
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
