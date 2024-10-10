{
  description = "A Clojure dialect hosted on LLVM with native C++ interop";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    flake-compat.url = "github:edolstra/flake-compat";
    treefmt-nix.url = "github:numtide/treefmt-nix";
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      imports = [
        inputs.treefmt-nix.flakeModule
      ];

      perSystem =
        { pkgs, ... }:
        {
          treefmt = {
            projectRootFile = "flake.nix";
            programs.shellcheck.enable = true;
            programs.nixfmt.enable = true;
            programs.actionlint.enable = true;
            programs.clang-format.enable = true;
            programs.cljfmt.enable = true;

            settings.formatter = {
              cljfmt.includes = [
                "*.jank"
              ];
            };
          };

          devShells.default = pkgs.mkShell {
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
            shellHook = ''
              export CC="${pkgs.clang}/bin/clang"
              export CXX="${pkgs.clang}/bin/clang++"
            '';
          };
        };
    };
}
