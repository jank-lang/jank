<p align="center">
  <a title="msys2.github.io" href="https://msys2.github.io"><img src="https://img.shields.io/website.svg?label=msys2.github.io&longCache=true&style=flat-square&url=http%3A%2F%2Fmsys2.github.io%2Findex.html&logo=github"></a><!--
  -->
  <a title="Join the chat at https://gitter.im/msys2/msys2" href="https://gitter.im/msys2/msys2"><img src="https://img.shields.io/badge/chat-on%20gitter-4db797.svg?longCache=true&style=flat-square&logo=gitter&logoColor=e8ecef"></a><!--
  -->
  <a title="GitHub Actions" href="https://github.com/msys2/MINGW-packages/actions?query=workflow%3Amain"><img alt="'main' workflow Status" src="https://img.shields.io/github/actions/workflow/status/msys2/MINGW-packages/main.yml?branch=master&longCache=true&style=flat-square&label=build&logo=github"></a><!--
  -->
  <a title="Follow msys2org on Twitter" href="https://twitter.com/msys2org"><img src="https://img.shields.io/twitter/follow/msys2org?color=31A4F1&logo=twitter&logoColor=white&style=flat-square"></a><!--
  -->
  <a title="Follow msys2org on Mastodon" href="https://fosstodon.org/@msys2org"><img src="https://img.shields.io/mastodon/follow/109365079526574177?color=000197&domain=https%3A%2F%2Ffosstodon.org%2F&logo=mastodon&logoColor=white&style=flat-square"></a><!--
  -->
  <a title="Join the community on Discord" href="https://discord.com/invite/jPQdRdDcT9"><img src="https://img.shields.io/discord/792780131906617355?color=5865F2&label=Discord&logo=discord&logoColor=white&style=flat-square"></a><!--
  -->
</p>

# MINGW-packages

This repository contains package scripts for MinGW-w64 targets to build under MSYS2.

MSYS2 is an independent rewrite of MSYS providing a Unix-like environment and command-line interface for Windows making it possible to port software running on POSIX systems (such as Linux, BSD, and Unix systems) to Windows.

## Documentation
See the [MSYS2 website](https://www.msys2.org/).

## Using packages
The common way to use these packages are **pre-built** binary packages from the MSYS2 MINGW64 repo (which includes the binaries, libraries, headers, man pages), and install it on your machine, and build against those packages/libraries as you are porting/writing your software.

Details about this, including information about how to find the correct package, are found in the [MSYS2 documentation](https://www.msys2.org/docs/package-management/).  
Short summary:
 
Assuming you have a properly installed MSYS2 environment, you can install the pre-built binary package by using the following command from the bash prompt:
 ```
    pacman -S ${package-name}
 ```

Please note: Not all the packages in this repository are built and accessible from the MSYS2 MINGW64 repo right away. After merging changes to the git repository it can take a few days until compiled and built packages are accessible in the repo. Also for some packages you can find older versions in the repo if you need older version, for some packages you have only the most recent version.

---------------------------------------------

As an alternative you can download or clone the package folder with the scripts to your machine and you **build it for yourself**, in whatever version you like. 

 Assuming you have a properly installed MSYS2 environment and build tools, you can build any package using the following command:
 ```
    cd ${package-name}
    MINGW_ARCH=mingw64 makepkg-mingw -sLf
 ```
 After that you can install the freshly built package(s) with the following command:
 ```
    pacman -U ${package-name}*.pkg.tar.xz
 ```

## Creating packages
See the [MSYS2 documentation](https://www.msys2.org/wiki/Creating-Packages) for instructions and advice about creating MINGW-packages.

## License

MSYS2-packages is licensed under BSD 3-Clause "New" or "Revised" License.
A full copy of the license is provided in [LICENSE](LICENSE).
