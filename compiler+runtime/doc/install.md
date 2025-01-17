# Installing jank
If you're on any of the below systems, you can install jank using your system's
package manager.

## Homebrew (macOS or Linux)
```sh
brew install jank-lang/jank/jank
```

If you can an error about `git-lfs`, try the following:

```sh
git lfs install
sudo ln -s "$(which git-lfs)" "$(git --exec-path)/git-lfs"
```

## Ubuntu 24.04
There are pre-compiled binaries for Ubuntu 24.04, which are built to follow the
`main` branch. You can download a tarball with everything you need here: https://github.com/jank-lang/jank/releases/tag/latest

## Anything else
If nothing above matches what you have, you can still build jank by following
the docs [here](./build.md).
