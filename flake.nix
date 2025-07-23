{
  description = "Dev environment for jank";

  inputs = {
    flake-parts.url = "github:hercules-ci/flake-parts";
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      imports = [inputs.flake-parts.flakeModules.easyOverlay];
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];
      perSystem = {
        pkgs,
        self',
        system,
        ...
      }: let
        llvmSrc = pkgs.fetchgit {
          url = "https://github.com/jank-lang/llvm-project.git";
          rev = "jank";
          hash = "sha256-NnEYQVDHGZJGwS+NaSXiOpeZgDu7gmm6fxiNxt0YE2M=";
          fetchSubmodules = true;
        };
      in {
        _module.args.pkgs = import inputs.nixpkgs {
          inherit system;
          overlays = [
            (final: prev: {
              llvmPackages_git =
                prev.llvmPackages_git
                // {
                  clang = prev.llvmPackages_git.clang.override {
                    monorepoSrc = llvmSrc;
                  };
                  llvm = prev.llvmPackages_git.llvm.override {
                    monorepoSrc = llvmSrc;
                  };
                  libclang = prev.llvmPackages_git.libclang.overrideAttrs {
                    monorepoSrc = llvmSrc;
                  };
                };
            })
          ];
        };
        legacyPackages = pkgs;
        formatter = pkgs.alejandra;
        devShells.default = (pkgs.mkShell.override {inherit (pkgs.llvmPackages_git) stdenv;}) {
          packages = with pkgs; [
            # Nix LSP
            nixd

            ## Required tools.
            cmake
            ninja
            pkg-config
            llvmPackages_git.clang

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

            # For clangd
            llvmPackages_git.llvm
            llvmPackages_git.libclang

            # For clang-tidy.
            llvmPackages_git.clang-tools
            gdb
            clangbuildanalyzer
            openjdk

            ## Dev libs.
            doctest
          ];

          shellHook = ''
            export ASAN_OPTIONS=detect_leaks=0
          '';

          # Nix assumes fortification by default, but that fails with debug builds.
          # Since this shell is used for development, we disabled fortification. It's
          # still enabled for our release builds in build.nix.
          # https://github.com/NixOS/nixpkgs/issues/18995
          hardeningDisable = ["fortify"];
        };
      };
    };
}
