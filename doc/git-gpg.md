# Git commit signing

By default, anyone can make a commit with any email and name set up. To help
prevent fraudulent commits, every commit pushed to jank repos must be
signed by the author's GPG key. *If you already have a GPG key in working order,
skip to [enable signing](#enable-signing). If you already sign your commits, skip to
[here](#verifying-everything-is-working) to verify everything is working.*

## Generate your key

Follow this [Github guide](https://help.github.com/articles/generating-a-new-gpg-key/) to
generate a new key with email address you use for Github and export the key to
your Github account.

## Enable signing

Every commit must be signed, so it's easiest to just enable signing by
default. You can do that with the following:

```bash
git config --global commit.gpgsign true
```

## GPG passphrase prompt

If you'd like GPG to prompt you for your passphrase in your terminal, rather
than as a GUI popup, you can put this in your `~/.bashrc` (or similar):

```bash
# Always prompt for GPG password from terminal.
export GPG_TTY=$(tty)
```

## Verifying everything is working

You can download and run the following script to verify your set up is working.
Your session should look something like this:

```bash
# cd to your ~/projects or whatever directory you use for repos.

$ git clone git@github.com:jank-lang/jank.git
$ cd jank
$ nu ./bin/verify-git-gpg.nu
gpg: Signature made Tue 17 Apr 2018 02:19:21 PM PDT
gpg:                using RSA key FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
gpg:                issuer "sally@foo.com"
gpg: Good signature from "Sally Dev <sally@foo.com>" [ultimate]
```
