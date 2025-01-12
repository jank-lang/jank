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
        devShells.default = (pkgs.mkShell.override { stdenv = pkgs.llvmPackages.stdenv; }) {
          packages = with pkgs; [
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
            nixd
            shellcheck
            # For clangd
            llvm
            llvmPackages.libclang

            # Libs.
            boehmgc
            boost
            libzip
            openssl

            # Dev libs.
            doctest
            # For clang-tidy.
            llvmPackages.clang-tools
          ];
        };
      };
    };
}
