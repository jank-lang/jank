# Installation
jank has continuous builds for macOS, Ubuntu, and Arch. These builds are
bleeding edge and you're encouraged to update regularly. If you're on any of the
supported systems, you can install jank using your system's package manager. If
not, you can still [build jank yourself](https://github.com/jank-lang/jank/blob/main/compiler+runtime/doc/build.md).

## Homebrew (macOS, aarch64)
We have a binary jank package in brew, so installation is quick and easy.

```sh
brew install jank-lang/jank/jank
```


To update jank, you can run the following.

```bash
brew update
brew reinstall jank-lang/jank/jank
```

If you'd like to install from source using brew, you can use `jank-lang/jank/jank-git` instead.

> [!NOTE]
> We don't yet have x86 binaries in the Homebrew package. If you'd like to help
> with this, please reach out.

## Ubuntu Linux (24.04, 24.10, 25.04)
We have a binary jank package in our own repo, so installation is quick and easy.

```bash
sudo apt install -y curl gnupg
curl -s "https://ppa.jank-lang.org/KEY.gpg" | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/jank.gpg >/dev/null
sudo curl -s -o /etc/apt/sources.list.d/jank.list "https://ppa.jank-lang.org/jank.list"
sudo apt update
sudo apt install -y jank
```

To update jank, you can run the following.

```bash
sudo apt update
sudo apt reinstall jank
```

> [!NOTE]
> Older versions of Ubuntu, like 22.04, will not work with jank. This is because
> jank requires C++20 to work and the libstdc++ on those systems is too old.

## Arch Linux (AUR)
We have a binary jank package in AUR, so installation is quick and easy.

```bash
yay -S jank-bin
```

To update jank, you can run the following.

```bash
yay -Syy
yay -S jank-bin
```


If you'd like to install from source on Arch, you can install `jank-git` instead.

## Something else?
Don't see your preferred system here? [Help us with
packaging](https://github.com/jank-lang/jank)! We want jank to
be everywhere.
