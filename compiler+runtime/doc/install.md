# Installing jank
If you're on any of the below systems, you can install jank using your system's
package manager.

## Homebrew (macOS or Linux)
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

To update, just reinstall.

## Ubuntu Linux (24.04, 24.10)
```bash
sudo apt install -y curl gnupg
curl -s "https://jank-lang.github.io/ppa/KEY.gpg" | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/jank.gpg >/dev/null
sudo curl -s -o /etc/apt/sources.list.d/jank.list "https://jank-lang.github.io/ppa/jank.list"
sudo apt update
sudo apt install -y jank
```

To update, just reinstall.

## Arch Linux (AUR)
There is no binary AUR package yet, but there's a source package which builds from
git.

```bash
yay -S jank-git
```

To update, just reinstall.

## Anything else
If nothing above matches what you have, you can still build jank by following
the docs [here](./build.md).
