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

## Anything else
If nothing above matches what you have, you can still build jank by following
the docs [here](./build.md).
