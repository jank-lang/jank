with (import <nixpkgs> {});
mkShell
{
  buildInputs =
  [
    llvm
    clang
    cling
    meson
    ninja
    entr
    ccls
    boost
    gcovr
    lcov
  ];
  shellHook =
  ''
  function jank-watch-unit-tests
  { git ls-files -cdmo --exclude-standard | entr bash -c "./bin/run-unit-tests || true"; }
  export CC=clang
  export CXX=clang++
  export CLING_INCLUDE_PATH="${pkgs.cling.unwrapped}/include"
  export CLING_LIB_PATH="${pkgs.cling.unwrapped}/lib"
  '';
}
