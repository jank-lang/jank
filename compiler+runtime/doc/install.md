# Installing jank
If you're on any of the below systems, you can install jank using your system's
package manager.

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

## Ubuntu Linux (24.04, 24.10)
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
There is no binary AUR package yet, but there's a source package which builds from
git.

```bash
yay -S jank-git
```

### Update
```bash
yay -Syy
yay -S jank-git
```

## Anything else
If nothing above matches what you have, you can still build jank by following
the docs [here](./build.md).
