#!/usr/bin/env nu

let tmp = "/tmp/jank-book-publish"

^mdbook build
mv live $tmp

^git fetch --no-recurse-submodules
^git checkout gh-pages
^git merge origin/gh-pages

let github_token = ($env | get -i GITHUB_TOKEN | default "")
if $github_token != "" {
    ^git config --global user.email "github-actions@notadomain"
    ^git config --global user.name "Github Actions"
}

# We're currently in book/, so move to the repo root.
cd ..

# Remove all git-tracked files.
^git ls-files | lines | each { |file| rm -rf $file }
rm -rf theme

# Move all files from the temp build directory (including dot-files).
ls -a $tmp | get name | each { |f| mv $f . }

^git add .
^git commit -m "Automated publish" --allow-empty

if $github_token != "" {
    try { ^git remote remove ci-origin } catch { }
    ^git remote add ci-origin $"https://($github_token)@github.com/jank-lang/jank.git"
    ^git push ci-origin gh-pages
} else {
    ^git push origin gh-pages
}

^git checkout -
