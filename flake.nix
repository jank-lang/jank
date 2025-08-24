{
  description = "Dev environment for jank";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];
      perSystem = {
        pkgs,
        ...
      }: {
        legacyPackages = pkgs;
        formatter = pkgs.alejandra;
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            stdenv.cc.cc.lib

            ## Required tools.
            cmake
            ninja
            pkg-config
            (pkgs.callPackage ./llvm.nix { })

            ## Required libs.
            boehmgc
            libzip
            openssl

            ## Dev tools.
            babashka
            entr
            gcovr
            lcov
            git
            nixd
            shellcheck
            # For clangd & clang-tidy.
            clang-tools
            gdb
            clangbuildanalyzer
            openjdk

            ## Dev libs.
            doctest
          ];

          shellHook =
          ''
          export CC=clang
          export CXX=clang++
          export ASAN_OPTIONS=detect_leaks=0
          '';

          # Nix assumes fortification by default, but that fails with debug builds.
          # Since this shell is used for development, we disabled fortification. It's
          # still enabled for our release builds in build.nix.
          # https://github.com/NixOS/nixpkgs/issues/18995
          hardeningDisable = [ "fortify" ];
        };
      };
    };
}
