with (import <nixpkgs> {});
mkShell
{
  buildInputs =
  [
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
  '';
}
