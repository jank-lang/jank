with (import <nixpkgs> {});
mkShell
{
  buildInputs =
  [
    # Build deps.
    cling
    boost
    meson
    ninja

    # Dev tools.
    llvm # For clang-format
    entr
    ccls
    gcovr
    lcov
    git
  ];
  shellHook =
  ''
  export CLING_DEV="${lib.getDev pkgs.cling.unwrapped}"
  export CC="clang"
  export CXX="clang++"
  export CLING_INCLUDE_PATH="${pkgs.cling.unwrapped}/include"
  export CLING_LIB_PATH="${pkgs.cling.unwrapped}/lib"
  export LLVM_INCLUDE_PATH="${lib.getDev pkgs.llvmPackages_5.llvm}/include"
  export LLVM_ROOT_PATH="${pkgs.cling.unwrapped}"
  '';
}
