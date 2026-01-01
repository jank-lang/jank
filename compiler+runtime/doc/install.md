# Installing jank
If you're on any of the below systems, you can install jank using your system's
package manager. If not, you can still [build jank yourself](https://github.com/jank-lang/jank/blob/main/compiler+runtime/doc/build.md).

## Homebrew (macOS or Linux)
### Install
There is no binary package for Homebrew yet, but there's a source package which
builds from git.

```sh
brew install jank-lang/jank/jank
```

If you get an error about `git-lfs`, try the following:

```sh
git lfs install
sudo ln -s "$(which git-lfs)" "$(git --exec-path)/git-lfs"
```

### Update
```bash
brew update
brew reinstall jank-lang/jank/jank
```

## Ubuntu Linux (24.04, 24.10, 25.04)
### Install
We have binary jank packages in our PPA, so installation is quick and easy.

```bash
sudo apt install -y curl gnupg
curl -s "https://ppa.jank-lang.org/KEY.gpg" | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/jank.gpg >/dev/null
sudo curl -s -o /etc/apt/sources.list.d/jank.list "https://ppa.jank-lang.org/jank.list"
sudo apt update
sudo apt install -y jank
```

### Update
```bash
sudo apt update
sudo apt reinstall jank
```

## Arch Linux (AUR)
### Install
There are two packages available in the AUR:

- `jank-git`: Builds jank directly from the latest source code on GitHub.
- `jank-bin`: Installs a precompiled binary from the official Debian package (recommended for most users).

To install:

```bash
# Build from source (requires clang, llvm, cmake, ...)
yay -S jank-git

# Install prebuilt binary (faster, no build deps)
yay -S jank-bin
```

### Update
```bash
yay -Syy
yay -S jank-git   # if you installed jank-git
yay -S jank-bin   # if you installed jank-bin
```

## Nix
### Usage
We have binary packages in our cachix cache, so usage via nix flakes is quick
and easy. You may also skip the cache setup and artifacts will be built from
source.

```bash
# Make sure cachix is installed, then follow the prompts.
cachix use jank-lang
```

Next, you can directly run the jank binary from the nix flake derivation. This
will always run from the latest main branch, unless a specific ref or rev is
given. For example, to run the `jank check-health` subcommand:

```bash
nix run git+https://github.com/jank-lang/jank.git -- check-health
```

You can also include jank as a flake input to your NixOS system configuration or
project. In this case, the jank version will be managed by the `flake.lock`
file. Here's a minimal example of such a nix flake:

```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    jank-lang.url = "git+https://github.com/jank-lang/jank.git";
  };
  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];
      perSystem = {
        inputs',
        pkgs,
        system,
        ...
      }: {
        devShells.default = pkgs.mkShell {
          packages = [
            inputs'.jank-lang.packages.default
          ];
        };
      };
    };
}
```

## Anything else
If nothing above matches what you have, you can still build jank by following
the docs [here](./build.md).
